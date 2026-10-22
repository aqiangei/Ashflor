#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticText.hh>

#include <Client/StaticData.hh>
#include <Client/I18n.hh>

#include <Client/Game.hh>

#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>

#include <format>
#include <string>
#include <vector>

using namespace Ui;

// Reverse lookup: drop sources / craft / shop / rarity merge for petal tooltips.
static std::string petal_acquisition_hint(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return {};

    std::vector<std::string> parts;

    std::vector<MobID::T> bosses;
    std::vector<MobID::T> trash;
    for (MobID::T m = 0; m < MobID::kNumMobs; ++m) {
        for (PetalID::T d : MOB_DATA[m].drops) {
            if (d != id) continue;
            if (is_boss_mob(m)) bosses.push_back(m);
            else trash.push_back(m);
            break;
        }
    }
    if (!bosses.empty() || !trash.empty()) {
        std::string line = I18n::tr(I18n::kAcquireDrop);
        std::vector<MobID::T> shown;
        shown.reserve(3);
        for (MobID::T m : bosses) {
            if (shown.size() >= 3) break;
            shown.push_back(m);
        }
        for (MobID::T m : trash) {
            if (shown.size() >= 3) break;
            shown.push_back(m);
        }
        char const *sep = I18n::use_chinese ? "、" : ", ";
        for (size_t i = 0; i < shown.size(); ++i) {
            if (i) line += sep;
            line += I18n::mob_name(shown[i]);
        }
        size_t total = bosses.size() + trash.size();
        if (total > shown.size())
            line += I18n::use_chinese ? " 等" : "…";
        parts.push_back(std::move(line));
    }

    if (Recipe::find_by_result(id) != (uint16_t)-1)
        parts.push_back(I18n::tr(I18n::kAcquireCraft));

    if (Craft::is_shop_purchasable(id))
        parts.push_back(I18n::tr(I18n::kAcquireShop));

    // Starter Basic (Common) — no drop table, given on spawn.
    if (id == PetalID::kBasic)
        parts.push_back(I18n::tr(I18n::kAcquireStarter));

    if (parts.empty()) return {};
    std::string out = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        out += " · ";
        out += parts[i];
    }
    return out;
}

Element *Ui::UiLoadout::petal_tooltips[PetalID::kNumPetals] = {nullptr};

static float get_reload_factor() {
    if (!Game::alive()) return 1;
    float factor = 1;
    Entity &player = Game::simulation.get_ent(Game::player_id);
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        factor *= PETAL_DATA[player.get_loadout_ids(i)].attributes.extra_reload_factor;
    }
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

static Ui::Element *make_petal_stat_container(PetalID::T id) {
    std::vector<Ui::Element *> stats = {new Ui::Element(0,10)};
    struct PetalData const &petal_data = PETAL_DATA[id];
    struct PetalAttributes const &attrs = petal_data.attributes;
    if (petal_data.health > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHealth); }, { .fill = 0xff77ff77 }),
            new Ui::StaticText(12, format_number(petal_data.health))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (petal_data.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kDamage); }, { .fill = 0xffff7777 }),
            new Ui::DynamicText(12, [&](){
                return format_number(petal_data.damage * get_damage_factor());
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.armor > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kArmor); }, { .fill = 0xff777777 }),
            new Ui::StaticText(12, format_number(attrs.armor))
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
            new Ui::StaticText(12, format_number(attrs.constant_heal) + "/s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.burst_heal > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHeal); }, { .fill = 0xffff96cb }),
            new Ui::StaticText(12, format_number(attrs.burst_heal))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.poison_damage.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoison); }, { .fill = 0xffce76db }),
            new Ui::StaticText(12, format_number(attrs.poison_damage.time * attrs.poison_damage.damage) + " (" + format_number(attrs.poison_damage.damage) + "/s)")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.burn_damage.damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kBurn); }, { .fill = 0xffff8844 }),
            new Ui::StaticText(12, format_number(attrs.burn_damage.time * attrs.burn_damage.damage) + " (" + format_number(attrs.burn_damage.damage) + "/s)")
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
            new Ui::StaticText(12, format_number(attrs.extra_health))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_body_damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kBodyDamage); }, { .fill = 0xffff7777 }),
            new Ui::StaticText(12, format_number(attrs.extra_body_damage))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.poison_armor > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kPoisonArmor); }, { .fill = 0xffce76db }),
            new Ui::StaticText(12, format_number(attrs.poison_armor) + "/s")
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_rotation_speed > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kRotationSpeed); }, { .fill = 0xffcde23b }),
            new Ui::StaticText(12, "+" + format_number(attrs.extra_rotation_speed) + " rad/s")
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
            new Ui::StaticText(12, format_pct(100 / attrs.vision_factor))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.move_speed_factor != 1) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kMoveSpeed); }, { .fill = 0xff3ecf8e }),
            new Ui::StaticText(12, format_pct(attrs.move_speed_factor * 100))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.magnet_radius > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "磁力范围：" : "Magnet Range:";
            }, { .fill = 0xff4a7fd9 }),
            new Ui::StaticText(12, format_number(attrs.magnet_radius))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_range > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kAttackRange); }, { .fill = 0xffcde23b }),
            new Ui::StaticText(12, "+" + format_number(attrs.extra_range))
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
            new Ui::StaticText(12, format_number(attrs.aura_heal) + "/s")
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
            new Ui::StaticText(12, format_number(attrs.splash_damage)
                + (attrs.splash_radius > 0
                    ? (" (" + format_number(attrs.splash_radius) + ")")
                    : std::string{}))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.radial_pulse_damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "脉冲伤害：" : "Pulse Dmg:";
            }, { .fill = 0xffffaa55 }),
            new Ui::StaticText(12, format_number(attrs.radial_pulse_damage)
                + (attrs.radial_pulse_radius > 0
                    ? (" (" + format_number(attrs.radial_pulse_radius) + ")")
                    : std::string{}))
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
            new Ui::StaticText(12, "+"+format_pct(100 * (attrs.extra_damage_factor - 1)))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.extra_reload_factor > 1.0001f) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kExtraReload); }, { .fill = 0xff7777ff }),
            new Ui::StaticText(12, "+"+format_pct(100 * (attrs.extra_reload_factor - 1)))
        }, 0, 5, { .h_justify = Style::Left }));
    } else if (attrs.extra_reload_factor < 0.999f) {
        // Cooldown cut (e.g. Golden Leaf); stacks by multiplying factors.
        float cut_pct = 100.f * (1.f - attrs.extra_reload_factor);
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? "花瓣冷却：" : "Cooldown:";
            }, { .fill = 0xff77ddff }),
            new Ui::StaticText(12, "-"+format_pct(cut_pct))
        }, 0, 5, { .h_justify = Style::Left }));
    }
    return new Ui::VContainer(stats, 0, 2, { .h_justify = Style::Left });
}

static void make_petal_tooltip(PetalID::T id) {
    Element *tooltip = new Ui::VContainer({
        new Ui::HFlexContainer(
            new Ui::DynamicText(20, [id](){ return I18n::petal_name(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
            new Ui::DynamicText(16, [=](){
                float reload = PETAL_DATA[id].reload * get_reload_factor();
                if (reload <= 0.f) return std::string{};
                if (PETAL_DATA[id].attributes.secondary_reload == 0)
                    return std::format("{:.1f}s ⟳", reload);
                return std::format("{:.1f} + {:.1f}s ⟳", reload,
                    PETAL_DATA[id].attributes.secondary_reload);
            }, { .fill = 0xffffffff, .v_justify = Style::Top }),
            5, 10, {}
        ),
        new Ui::DynamicText(14, [id](){ return I18n::rarity_name(PETAL_DATA[id].rarity); }, { .fill = RARITY_COLORS[PETAL_DATA[id].rarity], .h_justify = Style::Left }),
        new Ui::Element(0, 6),
        new Ui::DynamicText(12, [id](){ return I18n::petal_desc(id); }, { .fill = 0xffffffff, .h_justify = Style::Left }),
        new Ui::DynamicText(11, [id](){ return petal_acquisition_hint(id); }, { .fill = 0xff9ec9a8, .h_justify = Style::Left }),
        new Ui::Element(0, 6),
        make_petal_stat_container(id)
    }, 5, 2);
    tooltip->style.fill = 0xb01c2418;
    tooltip->style.round_radius = 12;
    tooltip->refactor();
    Ui::UiLoadout::petal_tooltips[id] = tooltip;
}

void Ui::make_petal_tooltips() {
    for (PetalID::T i = 0; i < PetalID::kNumPetals; ++i)
        make_petal_tooltip(i);
}