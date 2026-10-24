#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/StaticData.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Client/Game.hh>

#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Math.hh>

#include <algorithm>
#include <cstring>
#include <format>
#include <string>
#include <vector>

using namespace Ui;

// Drop tables store family seeds; match this rarity rung or same English name.
static uint8_t drop_seed_matches_petal(PetalID::T seed, PetalID::T want) {
    if (seed == PetalID::kNone || want == PetalID::kNone) return 0;
    if (seed == want) return 1;
    if (seed >= PetalID::kNumPetals || want >= PetalID::kNumPetals) return 0;
    uint8_t wr = PETAL_DATA[want].rarity;
    PetalID::T at = Craft::petal_at_rarity(seed, wr);
    if (at == want) return 1;
    char const *a = PETAL_DATA[seed].name;
    char const *b = PETAL_DATA[want].name;
    return a && b && std::strcmp(a, b) == 0;
}

// Reverse lookup: drop sources (with %) / craft mats / shop / starter.
static std::string petal_acquisition_hint(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return {};

    std::vector<std::string> parts;
    uint8_t view_r = PETAL_DATA[id].rarity;

    struct DropSrc { MobID::T mob; float pct; uint8_t boss; };
    std::vector<DropSrc> srcs;
    for (MobID::T m = 0; m < MobID::kNumMobs; ++m) {
        auto const &drops = MOB_DATA[m].drops;
        if (drops.size() == 0) continue;
        float weights[MAX_DROPS_PER_MOB] = {};
        float total = Craft::mob_loot_weights(m, view_r, weights);
        if (total <= 0.f) continue;
        float best = 0.f;
        for (uint32_t i = 0; i < drops.size(); ++i) {
            if (!drop_seed_matches_petal(drops[i], id)) continue;
            float pct = weights[i] / total * 100.f;
            if (pct > best) best = pct;
        }
        if (best > 0.f)
            srcs.push_back({ m, best, is_boss_mob(m) });
    }
    if (!srcs.empty()) {
        std::sort(srcs.begin(), srcs.end(), [](DropSrc const &a, DropSrc const &b) {
            if (a.boss != b.boss) return a.boss > b.boss;
            return a.pct > b.pct;
        });
        std::string line = I18n::tr(I18n::kAcquireDrop);
        constexpr size_t kShow = 8;
        char const *sep = I18n::use_chinese ? "、" : ", ";
        size_t n = srcs.size() < kShow ? srcs.size() : kShow;
        for (size_t i = 0; i < n; ++i) {
            if (i) line += sep;
            line += I18n::mob_name(srcs[i].mob);
            line += format_pct(srcs[i].pct);
        }
        if (srcs.size() > n) {
            line += I18n::use_chinese
                ? std::format(" 等{}种", srcs.size())
                : std::format(" +{}", srcs.size() - n);
        }
        parts.push_back(std::move(line));
    }

    {
        uint16_t ri = Recipe::find_by_result(id);
        if (ri != (uint16_t) -1) {
            Recipe::Def const &rec = Recipe::recipe_at(ri);
            std::string line = I18n::tr(I18n::kAcquireCraft);
            if (rec.ingredient_count > 0) {
                line += I18n::use_chinese ? "：" : ": ";
                char const *sep = I18n::use_chinese ? "、" : ", ";
                for (uint8_t i = 0; i < rec.ingredient_count; ++i) {
                    if (i) line += sep;
                    Recipe::Ingredient const &ing = rec.ingredients[i];
                    line += I18n::petal_name(ing.id);
                    if (ing.count > 1)
                        line += std::format("×{}", ing.count);
                }
            }
            parts.push_back(std::move(line));
        } else if (PETAL_DATA[id].rarity > 0) {
            // Most petals climb the rarity ladder via 5→1 craft.
            PetalID::T lower = Craft::petal_at_rarity(id, (uint8_t) (PETAL_DATA[id].rarity - 1));
            if (lower != PetalID::kNone && lower != id && Craft::can_upgrade_petal(lower))
                parts.push_back(I18n::tr(I18n::kAcquireMerge));
        }
    }

    if (Craft::is_shop_purchasable(id))
        parts.push_back(I18n::tr(I18n::kAcquireShop));

    // Starter Basic (Common) — no drop table, given on spawn.
    if (id == PetalID::kBasic)
        parts.push_back(I18n::tr(I18n::kAcquireStarter));

    if (parts.empty()) {
        return I18n::use_chinese
            ? std::string("获取途径：暂无固定掉落（可能来自特殊事件/商店）")
            : std::string("Source: no fixed mob drop (event/shop?)");
    }
    std::string out = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        out += " · ";
        out += parts[i];
    }
    return out;
}

Element *Ui::UiLoadout::petal_tooltips[PetalID::kNumPetals] = {nullptr};
Element *Ui::UiLoadout::petal_tooltips_leveled[PetalID::kNumPetals] = {nullptr};

static float get_reload_factor() {
    // Matches server: product of loadout extra_reload_factor (Golden Leaf etc.)
    // then talent cooldown mult — both stack multiplicatively.
    float factor = 1.f;
    if (Game::alive()) {
        Entity &player = Game::simulation.get_ent(Game::player_id);
        for (uint32_t i = 0; i < player.get_loadout_count(); ++i)
            factor *= PETAL_DATA[player.get_loadout_ids(i)].attributes.extra_reload_factor;
    } else {
        for (uint32_t i = 0; i < Game::loadout_count; ++i) {
            PetalID::T id = Game::cached_loadout[i];
            if (id != PetalID::kNone && id < PetalID::kNumPetals)
                factor *= PETAL_DATA[id].attributes.extra_reload_factor;
        }
    }
    factor *= Talent::reload_mult(Game::talent_ranks[TalentID::kCooldown]);
    // Match server Golden Leaf CDR cap (99%).
    if (factor < Craft::kReloadFactorFloor) factor = Craft::kReloadFactorFloor;
    if (factor > Craft::kReloadFactorCeil) factor = Craft::kReloadFactorCeil;
    return factor;
}

static float get_damage_factor() {
    if (!Game::alive()) return 1;
    float factor = 1;
    Entity &player = Game::simulation.get_ent(Game::player_id);
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        factor *= PETAL_DATA[player.get_loadout_ids(i)].attributes.extra_damage_factor;
    }
    return factor;
}

// Hold G: tip shows live loadout/talent/leaf multipliers. Default = base stats.
static uint8_t tooltip_show_buffs() {
    return Input::keys_held.contains('G') ? 1 : 0;
}

static float tooltip_dmg_mult() {
    return tooltip_show_buffs() ? get_damage_factor() : 1.f;
}

static float tooltip_reload_mult() {
    return tooltip_show_buffs() ? get_reload_factor() : 1.f;
}

static float tooltip_petal_hp_mult() {
    if (!tooltip_show_buffs()) return 1.f;
    return Talent::petal_health_mult(Game::talent_ranks[TalentID::kPetalArmor]);
}

static float tooltip_level_mult(PetalID::T id, uint8_t apply_level) {
    if (!apply_level || id == PetalID::kNone || id >= PetalID::kNumPetals) return 1.f;
    return Craft::level_stat_mult(Game::petal_levels[id]);
}

static Ui::Element *make_petal_stat_container(PetalID::T id, uint8_t apply_level) {
    std::vector<Ui::Element *> stats = {new Ui::Element(0,10)};
    struct PetalData const &petal_data = PETAL_DATA[id];
    struct PetalAttributes const &attrs = petal_data.attributes;
    if (petal_data.health > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHealth); }, { .fill = 0xff77ff77 }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].health * tooltip_level_mult(id, apply_level)
                    * tooltip_petal_hp_mult());
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (petal_data.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kDamage); }, { .fill = 0xffff7777 }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].damage * tooltip_dmg_mult()
                    * tooltip_level_mult(id, apply_level));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.armor > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kArmor); }, { .fill = 0xff777777 }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.armor
                    * tooltip_level_mult(id, apply_level));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.damage_reflection > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kDamageReflection); }, { .fill = 0xff777777 }),
            new Ui::StaticText(12, format_pct(attrs.damage_reflection * 100))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.constant_heal > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHeal); }, { .fill = 0xffff96cb }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.constant_heal
                    * tooltip_level_mult(id, apply_level)) + "/s";
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.burst_heal > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHeal); }, { .fill = 0xffff96cb }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.burst_heal
                    * tooltip_level_mult(id, apply_level));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.poison_damage.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoison); }, { .fill = 0xffce76db }),
            new Ui::DynamicText(12, [id, apply_level](){
                auto const &p = PETAL_DATA[id].attributes.poison_damage;
                float m = tooltip_level_mult(id, apply_level);
                return format_number(p.time * p.damage * m) + " ("
                    + format_number(p.damage * m) + "/s)";
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.burn_damage.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kBurn); }, { .fill = 0xffff8844 }),
            new Ui::DynamicText(12, [id, apply_level](){
                auto const &b = PETAL_DATA[id].attributes.burn_damage;
                float m = tooltip_level_mult(id, apply_level);
                return format_number(b.time * b.damage * m) + " ("
                    + format_number(b.damage * m) + "/s)";
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.armor_penetration > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kArmorPen); }, { .fill = 0xffaaaaaa }),
            new Ui::StaticText(12, format_pct(attrs.armor_penetration * 100))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.clump_radius > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kClump); }, { .fill = 0xffcde23b }),
            new Ui::StaticText(12, format_number(attrs.clump_radius))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.rotation_style == PetalAttributes::kFollowRot) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kFollowAim); }, { .fill = 0xffbbbbbb })
        }, 0, 5, { .h_justify = Style::Left }));
    } else if (attrs.rotation_style == PetalAttributes::kNoRot) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kNoRotate); }, { .fill = 0xffbbbbbb })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.split_projectile) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kSplitShot); }, { .fill = 0xffd2eb34 })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_health > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kFlowerHealth); }, { .fill = 0xff77ff77 }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.extra_health
                    * tooltip_level_mult(id, apply_level));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_body_damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kBodyDamage); }, { .fill = 0xffff7777 }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.extra_body_damage
                    * tooltip_level_mult(id, apply_level));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.poison_armor > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoisonArmor); }, { .fill = 0xffce76db }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.poison_armor
                    * tooltip_level_mult(id, apply_level)) + "/s";
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_rotation_speed > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kRotationSpeed); }, { .fill = 0xffcde23b }),
            new Ui::DynamicText(12, [id, apply_level](){
                float v = PETAL_DATA[id].attributes.extra_rotation_speed
                    * tooltip_level_mult(id, apply_level);
                return "+" + format_number(v) + " rad/s";
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.spawns != MobID::kNumMobs && attrs.spawn_count == 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kContents); }, { .fill = 0xffd2eb34 }),
            new Ui::DynamicText(12, [=](){
                return (petal_data.count > 1 ? format_number(petal_data.count) + "x " : "") + I18n::mob_name(attrs.spawns);
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.spawns != MobID::kNumMobs && attrs.spawn_count > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kSpawns); }, { .fill = 0xffd2eb34 }),
            new Ui::DynamicText(12, [=](){
                return format_number(attrs.spawn_count) + "x " + I18n::mob_name(attrs.spawns);
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.vision_factor < 1) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraVision); }, { .fill = 0xffcde23b }),
            new Ui::DynamicText(12, [id, apply_level](){
                float f = Craft::level_scale_factor(
                    PETAL_DATA[id].attributes.vision_factor,
                    tooltip_level_mult(id, apply_level));
                return format_pct(100 / f);
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.move_speed_factor != 1) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kMoveSpeed); }, { .fill = 0xff3ecf8e }),
            new Ui::DynamicText(12, [id, apply_level](){
                float f = Craft::level_scale_factor(
                    PETAL_DATA[id].attributes.move_speed_factor,
                    tooltip_level_mult(id, apply_level));
                return format_pct(f * 100);
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.magnet_radius > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "磁力范围：" : "Magnet Range:";
            }, { .fill = 0xff4a7fd9 }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.magnet_radius
                    * tooltip_level_mult(id, apply_level));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_range > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kAttackRange); }, { .fill = 0xffcde23b }),
            new Ui::DynamicText(12, [id, apply_level](){
                return "+" + format_number(PETAL_DATA[id].attributes.extra_range
                    * tooltip_level_mult(id, apply_level));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.slow_inflict_seconds > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "减速：" : "Slow:";
            }, { .fill = 0xff7ec8e3 }),
            new Ui::StaticText(12, format_number(attrs.slow_inflict_seconds) + "s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.dandelion_inflict_seconds > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "禁疗：" : "Anti-heal:";
            }, { .fill = 0xffc4a35a }),
            new Ui::StaticText(12, format_number(attrs.dandelion_inflict_seconds) + "s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    // Ground aura (Monstera / World Tree / Chrono…): heal, range, duration
    if (attrs.aura_heal > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "光环治疗：" : "Aura Heal:";
            }, { .fill = 0xff6fdc8c }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.aura_heal
                    * tooltip_level_mult(id, apply_level)) + "/s";
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.aura_radius > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "光环范围：" : "Aura Range:";
            }, { .fill = 0xff6fdc8c }),
            new Ui::StaticText(12, format_number(attrs.aura_radius))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.aura_duration > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "光环持续：" : "Aura Duration:";
            }, { .fill = 0xff6fdc8c }),
            new Ui::StaticText(12, format_number(attrs.aura_duration) + "s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.aura_slow_seconds > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "光环减速：" : "Aura Slow:";
            }, { .fill = 0xff7ec8e3 }),
            new Ui::StaticText(12, format_number(attrs.aura_slow_seconds) + "s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.splash_damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "溅射伤害：" : "Splash Dmg:";
            }, { .fill = 0xffff9966 }),
            new Ui::DynamicText(12, [id, apply_level](){
                auto const &a = PETAL_DATA[id].attributes;
                return format_number(a.splash_damage * tooltip_level_mult(id, apply_level))
                    + (a.splash_radius > 0
                        ? (" (" + format_number(a.splash_radius) + ")")
                        : std::string{});
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.radial_pulse_damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "脉冲伤害：" : "Pulse Dmg:";
            }, { .fill = 0xffffaa55 }),
            new Ui::DynamicText(12, [id, apply_level](){
                auto const &a = PETAL_DATA[id].attributes;
                return format_number(a.radial_pulse_damage * tooltip_level_mult(id, apply_level))
                    + (a.radial_pulse_radius > 0
                        ? (" (" + format_number(a.radial_pulse_radius) + ")")
                        : std::string{});
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.snare_seconds > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "定身：" : "Snare:";
            }, { .fill = 0xffc9a227 }),
            new Ui::StaticText(12, format_number(attrs.snare_seconds) + "s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.execute_threshold > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "斩杀：" : "Execute:";
            }, { .fill = 0xffff5577 }),
            new Ui::StaticText(12,
                format_pct(attrs.execute_threshold * 100) + " HP → ×"
                + format_number(attrs.execute_factor))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.taunt_radius > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "嘲讽范围：" : "Taunt Range:";
            }, { .fill = 0xffe0a060 }),
            new Ui::StaticText(12, format_number(attrs.taunt_radius))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.on_hit_heal > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "击中回血：" : "On-hit Heal:";
            }, { .fill = 0xffff96cb }),
            new Ui::StaticText(12, format_number(attrs.on_hit_heal))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.bonus_vs_poisoned > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "对中毒加成：" : "Vs Poisoned:";
            }, { .fill = 0xffce76db }),
            new Ui::StaticText(12, "+" + format_pct(attrs.bonus_vs_poisoned * 100))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.bonus_vs_slowed > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "对减速加成：" : "Vs Slowed:";
            }, { .fill = 0xff7ec8e3 }),
            new Ui::StaticText(12, "+" + format_pct(attrs.bonus_vs_slowed * 100))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.hit_cycle > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "连击强化：" : "Hit Cycle:";
            }, { .fill = 0xffffcc66 }),
            new Ui::StaticText(12,
                "×" + format_number(attrs.hit_cycle_factor)
                + " /" + format_number((float) attrs.hit_cycle))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    // Non-default mass (Honey / Meteor ground props)
    if (attrs.mass > 0.1001f || attrs.mass < 0.0999f) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "质量：" : "Mass:";
            }, { .fill = 0xffbbbbbb }),
            new Ui::StaticText(12, format_number(attrs.mass))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.defend_only) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "仅防御态生效" : "Defend only";
            }, { .fill = 0xffaaaaaa })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_damage_factor > 1) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraDamage); }, { .fill = 0xffff7777 }),
            new Ui::DynamicText(12, [id, apply_level](){
                float f = Craft::level_scale_factor(
                    PETAL_DATA[id].attributes.extra_damage_factor,
                    tooltip_level_mult(id, apply_level));
                return "+"+format_pct(100 * (f - 1));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_reload_factor > 1.0001f) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraReload); }, { .fill = 0xff7777ff }),
            new Ui::DynamicText(12, [id, apply_level](){
                float f = Craft::level_scale_factor(
                    PETAL_DATA[id].attributes.extra_reload_factor,
                    tooltip_level_mult(id, apply_level));
                return "+"+format_pct(100 * (f - 1));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    } else if (attrs.extra_reload_factor < 0.999f) {
        // Cooldown cut (e.g. Golden Leaf); stacks by multiplying factors.
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "花瓣冷却：" : "Cooldown:";
            }, { .fill = 0xff77ddff }),
            new Ui::DynamicText(12, [id, apply_level](){
                float f = Craft::level_scale_factor(
                    PETAL_DATA[id].attributes.extra_reload_factor,
                    tooltip_level_mult(id, apply_level));
                return "-"+format_pct(100.f * (1.f - f));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    return new Ui::VContainer(stats, 0, 2, { .h_justify = Style::Left });
}

static Element *make_petal_tooltip_body(PetalID::T id, uint8_t apply_level) {
    Element *tooltip = new Ui::VContainer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(20, [id](){ return I18n::petal_name(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
            new Ui::DynamicText(16, [=](){
                float reload = PETAL_DATA[id].reload * tooltip_reload_mult();
                if (reload <= 0.f) return std::string{};
                if (PETAL_DATA[id].attributes.secondary_reload == 0)
                    return std::format("{:.1f}s ⟳", reload);
                return std::format("{:.1f} + {:.1f}s ⟳", reload,
                    PETAL_DATA[id].attributes.secondary_reload);
            }, { .fill = 0xffffffff, .v_justify = Style::Top }),
            5, 10, {}
        ),
        new Ui::DynamicText(14, [id, apply_level](){
            uint8_t r = PETAL_DATA[id].rarity;
            if (apply_level && r == RarityID::kUnique && Game::petal_levels[id] > 0)
                return std::format("{} · Lv.{}", I18n::rarity_name(r), Game::petal_levels[id]);
            return std::string{I18n::rarity_name(r)};
        }, { .fill = RARITY_COLORS[PETAL_DATA[id].rarity], .h_justify = Style::Left }),
        new Ui::DynamicText(11, [](){
            if (tooltip_show_buffs())
                return I18n::use_chinese
                    ? std::string{"【加成后】金叶冷却 · 装备伤害 · 天赋花瓣耐久"}
                    : std::string{"[Buffed] leaf CDR · gear damage · talent petal HP"};
            return I18n::use_chinese
                ? std::string{"按住 G 显示加成后数值"}
                : std::string{"Hold G for buffed stats"};
        }, { .fill = 0xffc8e6a0, .h_justify = Style::Left }),
        new Ui::Element(0, 6),
        new Ui::DynamicText(12, [id](){ return I18n::petal_desc(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::DynamicText(12, [id](){ return petal_acquisition_hint(id); }, { .fill = 0xffa8e0b8, .h_justify = Style::Left }),
        new Ui::Element(0, 6),
        make_petal_stat_container(id, apply_level)
    }, 5, 2);
    tooltip->style.fill = 0xb01c2418;
    tooltip->style.round_radius = 12;
    tooltip->refactor();
    return tooltip;
}

void Ui::make_petal_tooltips() {
    for (PetalID::T i = 0; i < PetalID::kNumPetals; ++i) {
        Ui::UiLoadout::petal_tooltips[i] = make_petal_tooltip_body(i, 0);
        Ui::UiLoadout::petal_tooltips_leveled[i] = make_petal_tooltip_body(i, 1);
    }
}