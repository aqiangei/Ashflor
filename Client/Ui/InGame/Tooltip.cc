#include <Client/Ui/InGame/Loadout.hh>

#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/StaticText.hh>
#include <Client/Ui/LazyTooltip.hh>

#include <Client/StaticData.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>

#include <Client/Game.hh>

#include <Server/PetalMechanics.hh>

#include <Shared/Craft.hh>
#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Math.hh>

#include <algorithm>
#include <cmath>
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

// Title-screen / dead: Mimic left resolve against cached_loadout (same rules as
// Server/PetalMechanics.hh effective_petal_id).
static PetalID::T tip_cached_effective(uint32_t slot) {
    uint32_t n = Game::loadout_count;
    if (slot >= n) return PetalID::kNone;
    PetalID::T id = Game::cached_loadout[slot];
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return PetalID::kNone;
    if (!PETAL_DATA[id].attributes.mimic_left) return id;
    if (n < 2) return PetalID::kNone;
    PetalID::T left = Game::cached_loadout[slot == 0 ? n - 1 : slot - 1];
    if (left == PetalID::kNone || left >= PetalID::kNumPetals) return PetalID::kNone;
    if (PETAL_DATA[left].attributes.mimic_left || PETAL_DATA[left].attributes.corruption)
        return PetalID::kNone;
    for (int rarity = PETAL_DATA[id].rarity; rarity >= 0; --rarity) {
        PetalID::T target = Craft::petal_at_rarity(left, (uint8_t) rarity);
        if (target != PetalID::kNone && target < PetalID::kNumPetals) return target;
    }
    return PetalID::kNone;
}

static float get_reload_factor() {
    // Matches server Flower.cc: product of loadout extra_reload_factor (Golden Leaf
    // etc., Mimic-resolved) × Unique Lv on first primary copy × talent × Cogwheel,
    // then the global 99% floor.
    float factor = 1.f;
    uint8_t has_cogwheel = 0;
    if (Game::alive()) {
        Entity &player = Game::simulation.get_ent(Game::player_id);
        uint32_t n = player.get_loadout_count();
        for (uint32_t i = 0; i < n; ++i) {
            PetalID::T slot_id = player.get_loadout_ids(i);
            if (slot_id == PetalID::kNone || slot_id >= PetalID::kNumPetals) continue;
            PetalID::T id = effective_petal_id(player, i);
            if (id == PetalID::kNone || id >= PetalID::kNumPetals) continue;
            if (std::strcmp(PETAL_DATA[id].name, "Cogwheel") == 0) has_cogwheel = 1;
            float lv_mult = 1.f;
            if (Craft::can_level_petal(slot_id)) {
                uint8_t first = 1;
                for (uint32_t j = 0; j < i; ++j) {
                    if (player.get_loadout_ids(j) == slot_id) { first = 0; break; }
                }
                if (first)
                    lv_mult = Craft::level_stat_mult(
                        Craft::petal_level_get(Game::petal_levels, slot_id));
            }
            factor *= Craft::level_scale_factor(
                PETAL_DATA[id].attributes.extra_reload_factor, lv_mult);
        }
    } else {
        for (uint32_t i = 0; i < Game::loadout_count; ++i) {
            PetalID::T slot_id = Game::cached_loadout[i];
            if (slot_id == PetalID::kNone || slot_id >= PetalID::kNumPetals) continue;
            PetalID::T id = tip_cached_effective(i);
            if (id == PetalID::kNone || id >= PetalID::kNumPetals) continue;
            if (std::strcmp(PETAL_DATA[id].name, "Cogwheel") == 0) has_cogwheel = 1;
            float lv_mult = 1.f;
            if (Craft::can_level_petal(slot_id)) {
                uint8_t first = 1;
                for (uint32_t j = 0; j < i; ++j) {
                    if (Game::cached_loadout[j] == slot_id) { first = 0; break; }
                }
                if (first)
                    lv_mult = Craft::level_stat_mult(
                        Craft::petal_level_get(Game::petal_levels, slot_id));
            }
            factor *= Craft::level_scale_factor(
                PETAL_DATA[id].attributes.extra_reload_factor, lv_mult);
        }
    }
    factor *= Talent::reload_mult(Game::talent_ranks[TalentID::kCooldown]);
    if (has_cogwheel) factor *= 0.97f;
    if (factor < Craft::kReloadFactorFloor) factor = Craft::kReloadFactorFloor;
    if (factor > Craft::kReloadFactorCeil) factor = Craft::kReloadFactorCeil;
    return factor;
}

static float get_damage_factor() {
    if (!Game::alive()) return 1;
    float factor = 1;
    // Per English-name max amp (match server Flower.cc — no Skyblade^n).
    static constexpr uint32_t kAmpCap = 12;
    uint32_t amp_hash[kAmpCap] = {};
    float amp_best[kAmpCap] = {};
    uint32_t amp_n = 0;
    Entity &player = Game::simulation.get_ent(Game::player_id);
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        PetalID::T pid = player.get_loadout_ids(i);
        if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
        auto const &pd = PETAL_DATA[pid];
        float lv = Craft::level_stat_mult(Craft::petal_level_get(Game::petal_levels, pid));
        float f = Craft::level_scale_factor(pd.attributes.extra_damage_factor, lv);
        if (f > 1.f) {
            char const *nm = pd.name;
            uint32_t h = 2166136261u;
            for (char const *p = nm; p && *p; ++p) {
                h ^= (uint8_t)*p;
                h *= 16777619u;
            }
            uint32_t found = amp_n;
            for (uint32_t ai = 0; ai < amp_n; ++ai) {
                if (amp_hash[ai] == h) { found = ai; break; }
            }
            if (found < amp_n) {
                if (f > amp_best[found]) amp_best[found] = f;
            } else if (amp_n < kAmpCap) {
                amp_hash[amp_n] = h;
                amp_best[amp_n] = f;
                ++amp_n;
            }
        } else if (f > 0.f && f < 1.f) {
            factor *= f;
        }
    }
    float prod = 1.f;
    for (uint32_t ai = 0; ai < amp_n; ++ai) prod *= amp_best[ai];
    if (prod > 8.f) prod = 8.f;
    if (prod > 1.f) factor *= prod;
    return factor;
}

// Hold G: tip shows live loadout/talent/leaf multipliers. Default = base stats.
static uint8_t tooltip_show_buffs() {
    return Input::keys_held.contains('G') ? 1 : 0;
}

static float tooltip_dmg_mult() {
    return tooltip_show_buffs() ? get_damage_factor() : 1.f;
}

// Cooldown tip always mirrors combat (Golden Leaf / talent / Cogwheel).
// Damage & petal-HP buffs stay behind hold-G.
static float tooltip_reload_mult() {
    return get_reload_factor();
}

static float tooltip_petal_hp_mult() {
    if (!tooltip_show_buffs()) return 1.f;
    return Talent::petal_health_mult(Game::talent_ranks[TalentID::kPetalArmor]);
}

static float tooltip_level_mult(PetalID::T id, uint8_t apply_level) {
    if (!apply_level || id == PetalID::kNone || id >= PetalID::kNumPetals) return 1.f;
    return Craft::level_stat_mult(Craft::petal_level_get(Game::petal_levels, id));
}

static Ui::Element *make_petal_stat_container(PetalID::T id, uint8_t apply_level) {
    std::vector<Ui::Element *> stats = {new Ui::Element(0,10)};
    struct PetalData const &petal_data = PETAL_DATA[id];
    struct PetalAttributes const &attrs = petal_data.attributes;
    auto effect = [&](char const *zh, char const *en, std::string value) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [zh, en](){ return I18n::use_chinese ? zh : en; }, { .fill = 0xff9bdfcf }),
            new Ui::StaticText(12, value)
        }, 0, 5, { .h_justify = Style::Left }));
    };
    if (attrs.on_hit_heal_ratio > 0.f)
        effect("命中吸血：", "Lifesteal: ", format_pct(attrs.on_hit_heal_ratio * 100.f));
    if (attrs.aggro_shrink > 0.f)
        effect("怪物索敌范围：", "Mob aggro range: ", "-" + format_pct(std::fmin(.95f, attrs.aggro_shrink) * 100.f));
    if (attrs.attraction_radius > 0.f)
        effect("花瓣吸引（常驻，攻击环外额外）：", "Petal attraction (always on, extra past attack ring): ", format_number(attrs.attraction_radius));
    if (attrs.petal_repair > 0.f)
        effect("花瓣耐久回复：", "Petal durability restore: ", format_number(attrs.petal_repair) + "/s");
    if (attrs.body_radius_bonus > 0.f)
        effect("体型增大：", "Body size: ", "+" + format_number(attrs.body_radius_bonus));
    if (attrs.heal_amp > 0.f)
        effect("治疗放大：", "Heal amp: ", "+" + format_pct(attrs.heal_amp * 100.f));
    if (attrs.damage_absorb > 0.f)
        effect("伤害吸收：", "Damage absorb: ", format_number(attrs.damage_absorb));
    if (attrs.knockback_taken > 0.f)
        effect("受击击退：", "Knockback taken: ", "×" + format_number(attrs.knockback_taken));
    if (attrs.solo_damage_factor > 1.0001f)
        effect("独处伤害：", "Solo damage: ", "×" + format_number(attrs.solo_damage_factor));
    if (attrs.petal_evasion > 0.f)
        effect("花瓣闪避：", "Petal evasion: ", format_pct(attrs.petal_evasion * 100.f));
    if (attrs.flower_evasion > 0.f)
        effect("花朵闪避：", "Flower evasion: ", format_pct(attrs.flower_evasion * 100.f));
    if (attrs.petal_hp_factor > 1.0001f)
        effect("花瓣耐久倍率：", "Petal HP: ", "×" + format_number(attrs.petal_hp_factor));
    if (attrs.luck > 0.f)
        effect("幸运：", "Luck: ", "+" + format_number(attrs.luck));
    if (attrs.crit_chance > 0.f)
        effect("暴击：", "Crit: ", format_pct(attrs.crit_chance * 100.f) + " → ×" + format_number(attrs.crit_factor > 0.f ? attrs.crit_factor : 2.f));
    if (attrs.clay_max_damage > 0.f)
        effect("黏土蓄力上限：", "Clay charge cap: ", format_number(attrs.clay_max_damage));
    if (attrs.tomato_max_damage > 0.f)
        effect("番茄蓄力上限：", "Tomato charge cap: ", format_number(attrs.tomato_max_damage));
    if (attrs.domino_max_damage > 0.f)
        effect("骨牌蓄力上限：", "Domino charge cap: ", format_number(attrs.domino_max_damage));
    if (attrs.vs_projectile_factor > 1.0001f)
        effect("对弹射物：", "Vs projectiles: ", "×" + format_number(attrs.vs_projectile_factor));
    if (attrs.hit_interval_seconds > 0.f)
        effect("命中间隔：", "Hit interval: ", format_number(attrs.hit_interval_seconds) + "s");
    if (attrs.phase_through)
        effect("穿透：", "Phase through: ", std::to_string(attrs.phase_through));
    if (attrs.equipment == EquipmentFlags::kCutter)
        effect("穿怪：", "Phase mobs: ", I18n::use_chinese ? "花体可穿过怪物" : "body phases through mobs");
    if (attrs.fixed_orbit_range)
        effect("固定轨道半径：", "Fixed orbit range: ", "是");
    if (attrs.privet_extra_poison > 0.f)
        effect("女贞毒伤加成：", "Privet poison amp: ", "+" + format_pct(attrs.privet_extra_poison * 100.f));
    if (attrs.powder_max_rarity)
        effect("粉末压制稀有度上限：", "Powder max rarity: ", I18n::rarity_name(attrs.powder_max_rarity));
    if (attrs.mimic_left) effect("拟态左侧花瓣：", "Mimic left petal: ", "是");
    if (attrs.card_random_stats) effect("纸牌随机数值：", "Card random stats: ", "是");
    if (attrs.ankh_mark) effect("安卡标记：", "Ankh mark: ", "是");
    if (attrs.compass) effect("罗盘：", "Compass: ", "是");
    if (attrs.blood_sacrifice) effect("死亡召唤：", "Blood sacrifice: ", "是");
    if (attrs.champion_crown) effect("王者之冠：", "Champion crown: ", "是");
    if (attrs.soul_splitter) effect("灵魂分裂：", "Soul splitter: ", "是");
    if (attrs.paranormal) effect("超自然视野：", "Paranormal: ", "是");
    if (attrs.corruption) effect("腐化：", "Corruption: ", "是");
    if (attrs.dark_mark) effect("暗黑标记：", "Dark mark: ", "是");
    if (attrs.mana_gen > 0.f) effect("法力回复：", "Mana regen: ", format_number(attrs.mana_gen) + "/s");
    if (attrs.mana_cap > 0.f) effect("法力上限：", "Mana capacity: ", format_number(attrs.mana_cap));
    if (attrs.mana_spawn_cost > 0.f) effect("法力消耗：", "Mana cost: ", format_number(attrs.mana_spawn_cost));
    if (attrs.mana_absorb_per > 0.f) effect("每点法力抵伤：", "Damage per mana: ", format_number(attrs.mana_absorb_per));
    if (attrs.mana_lightning_per > 0.f) effect("每点法力雷伤：", "Lightning per mana: ", format_number(attrs.mana_lightning_per));
    if (attrs.lightning_damage > 0.f) effect("闪电伤害：", "Lightning: ", format_number(attrs.lightning_damage) + " × " + std::to_string(std::max(1, (int)attrs.lightning_chains)));
    if (attrs.armor_shred > 0.f) effect("削减护甲：", "Armor shred: ", format_number(attrs.armor_shred) + " / " + format_number(attrs.armor_shred_seconds) + "s");
    if (attrs.totem_heal > 0.f) effect("击杀后治疗：", "On-kill regeneration: ", format_number(attrs.totem_heal) + "/s · " + format_number(attrs.totem_seconds) + "s");
    if (attrs.damage_delay_seconds > 0.f) effect("伤害延迟结算：", "Damage deferred: ", format_number(attrs.damage_delay_seconds) + "s");
    if (attrs.death_delay_seconds > 0.f) effect("致死缓冲：", "Lethal grace: ", format_number(attrs.death_delay_seconds) + "s");
    if (attrs.nazar_charges) {
        effect("减伤次数：", "Mitigation charges: ", std::to_string(attrs.nazar_charges));
        if (attrs.nazar_mitigation > 0.f)
            effect("减伤比例：", "Mitigation: ", format_pct(attrs.nazar_mitigation * 100.f));
    }
    if (attrs.battery_charges) effect("反击次数：", "Discharge charges: ", std::to_string(attrs.battery_charges));
    if (attrs.meteor_size_factor > 1.001f)
        effect("陨石面积：", "Meteor size: ", "×" + format_number(attrs.meteor_size_factor));
    if (attrs.secondary_cdr_factor > 0.f && attrs.secondary_cdr_factor < 0.999f)
        effect("副冷却：", "Secondary CD: ", "×" + format_number(attrs.secondary_cdr_factor));
    if (attrs.wax_block) effect("飞弹格挡：", "Projectile blocks: ", "1");
    if (attrs.shovel_burrow) effect("按下防御遁地：", "Press defend to burrow: ", "0.6s / " + format_number(std::fmax(2.f, petal_data.reload)) + "s");
    if (attrs.root_armor > 0.f) effect("每层护甲：", "Armor per stack: ", format_number(attrs.root_armor) + " × " + std::to_string(attrs.root_max_stacks));
    if (attrs.overheal_to_shield > 0.f) effect("溢出治疗转护盾：", "Overheal to shield: ", format_pct(attrs.overheal_to_shield * 100.f));
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
                float dmg = PETAL_DATA[id].damage;
                float const stack = PETAL_DATA[id].attributes.stack_damage;
                // Triangle: tip damage follows equipped same-rarity copies (not max).
                if (stack > 0.f) {
                    uint32_t copies = 0;
                    if (Game::alive()) {
                        Entity &player = Game::simulation.get_ent(Game::player_id);
                        for (uint32_t i = 0; i < player.get_loadout_count(); ++i)
                            if (player.get_loadout_ids(i) == id) ++copies;
                    } else {
                        for (uint32_t i = 0; i < Game::loadout_count; ++i)
                            if (Game::cached_loadout[i] == id) ++copies;
                    }
                    if (copies >= 1)
                        dmg = PETAL_DATA[id].damage + stack * (float)(copies - 1);
                }
                return format_number(dmg * tooltip_dmg_mult()
                    * tooltip_level_mult(id, apply_level));
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.stack_damage > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){
                return I18n::use_chinese ? std::string{"叠加："} : std::string{"Stack:"};
            }, { .fill = 0xffffaa77 }),
            new Ui::DynamicText(12, [id](){
                float stack = PETAL_DATA[id].attributes.stack_damage;
                if (I18n::use_chinese)
                    return std::format("+{}/枚同稀有度", format_number(stack));
                return std::format("+{}/same-rarity copy", format_number(stack));
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
                    * tooltip_level_mult(id, apply_level)
                    * heal_tier_catchup(PETAL_DATA[id].rarity)) + "/s";
            })
        }, 0, 5, { .h_justify = Style::Left }));
    }
    if (attrs.burst_heal > 0) {
        stats.push_back(new Ui::HContainer({
            new Ui::DynamicText(12, [](){ return I18n::tr(I18n::kHeal); }, { .fill = 0xffff96cb }),
            new Ui::DynamicText(12, [id, apply_level](){
                return format_number(PETAL_DATA[id].attributes.burst_heal
                    * tooltip_level_mult(id, apply_level)
                    * heal_tier_catchup(PETAL_DATA[id].rarity));
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
                float f = Craft::level_scale_vision(
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
                    * tooltip_level_mult(id, apply_level)
                    * heal_tier_catchup(PETAL_DATA[id].rarity)) + "/s";
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
                float const rf = tooltip_reload_mult();
                float reload = PETAL_DATA[id].reload * rf;
                // Match Flower.cc ≥2-tick spawn floor (TPS=20 → 0.1s).
                float const floor_s = 2.f / (float) TPS;
                if (reload > 0.f && reload < floor_s) reload = floor_s;
                if (reload <= 0.f) return std::string{};
                float sec = PETAL_DATA[id].attributes.secondary_reload;
                if (sec <= 0.f)
                    return std::format("{:.1f}s ⟳", reload);
                float sec_s = sec * rf;
                if (sec_s < floor_s) sec_s = floor_s;
                return std::format("{:.1f} + {:.1f}s ⟳", reload, sec_s);
            }, { .fill = 0xffffffff, .v_justify = Style::Top }),
            5, 10, {}
        ),
        new Ui::DynamicText(14, [id, apply_level](){
            uint8_t r = PETAL_DATA[id].rarity;
            if (apply_level && r == RarityID::kUnique
                && Craft::petal_level_get(Game::petal_levels, id) > 0)
                return std::format("{} · Lv.{}", I18n::rarity_name(r),
                    Craft::petal_level_get(Game::petal_levels, id));
            return std::string{I18n::rarity_name(r)};
        }, { .fill = RARITY_COLORS[PETAL_DATA[id].rarity], .h_justify = Style::Left }),
        new Ui::DynamicText(11, [](){
            if (tooltip_show_buffs())
                return I18n::use_chinese
                    ? std::string{"【加成后】装备伤害 · 天赋花瓣耐久（冷却已含金叶）"}
                    : std::string{"[Buffed] gear damage · talent petal HP (CDR always on)"};
            return I18n::use_chinese
                ? std::string{"冷却已含金叶；按住 G 看伤害/耐久加成"}
                : std::string{"CDR includes leaf; hold G for damage/HP buffs"};
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
        Ui::UiLoadout::petal_tooltips[i] = new Ui::LazyTooltip([i]() { return make_petal_tooltip_body(i, 0); });
        Ui::UiLoadout::petal_tooltips_leveled[i] = new Ui::LazyTooltip([i]() { return make_petal_tooltip_body(i, 1); });
    }
}
