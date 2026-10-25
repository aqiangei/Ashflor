#include <Server/Process.hh>
#include <Server/PetalMechanics.hh>

#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>
#include <Server/Loot.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>

#include <array>
#include <cmath>
#include <cstring>

struct PlayerBuffs {
    float extra_rot = 0;
    float extra_range = 0;
    float heal = 0;
    float vision_factor = 1;
    float move_speed_factor = 1;
    float magnet_radius = 0;
    float attraction_radius = 0; // Lentil: sum of equipped attraction_radius
    float extra_health = 0;
    float relic_hp_frac = 0; // Mysterious Relic ships 0.1..0.9 of base HP
    uint8_t has_corruption = 0;
    uint8_t unique_corruption = 0;
    float body_radius_bonus = 0; // Soil etc. (sum)
    float air_body_radius = 0;   // Air: strongest only
    float heal_amp = 0;
    float luck = 0;
    float damage_absorb = 0;
    float flower_evasion = 0;
    float nazar_mitigation = 0;
    uint8_t nazar_charges = 0;
    float mana_gen = 0;
    float mana_cap = 0;
    float petal_hp_factor = 1;
    float petal_repair = 0; // Salve: HP/s to living loadout petals
    float extra_damage = 0;
    float damage_factor = 1;
    // Per English-name max amp, then product across names (stops Skyblade^n,
    // still lets Skyblade × other amp petals stack). Soft-capped below.
    static constexpr uint32_t kAmpNameCap = 12;
    uint32_t amp_name_hash[kAmpNameCap] = {};
    float amp_name_best[kAmpNameCap] = {};
    uint32_t amp_name_n = 0;
    float reload_factor = 1;
    float aggro_shrink = 0;
    uint8_t yinyang_count = 0;
    uint8_t is_poisonous = 0;
    uint8_t equip_flags = 0;
    // Stateful equipment effects
    float totem_heal = 0;
    float totem_seconds = 0;
    float root_armor = 0;
    float root_interval = 0;
    uint8_t root_max_stacks = 0;
    uint8_t ankh_mark = 0;
    uint8_t shovel_burrow = 0;
    uint8_t wax_block = 0;
    float shovel_reload = 2.f;
    float overheal_to_shield = 0;
    uint8_t compass = 0;
    uint8_t dark_mark = 0;
    uint8_t champion_crown = 0;
    uint8_t soul_splitter = 0;
    uint8_t paranormal = 0;
    uint8_t has_fragment = 0;
    uint8_t has_cogwheel = 0;
};

// Triangle: same-rarity copies add stack_damage each (incl. Unique+Unique).
static uint32_t _triangle_stack_sources(Entity const &player, PetalID::T id) {
    uint32_t n = 0;
    for (uint32_t si = 0; si < player.get_loadout_count(); ++si) {
        if (effective_petal_id(player, si) == id)
            ++n;
    }
    return n;
}

static float _triangle_base_damage(PetalID::T id, uint32_t sources) {
    struct PetalData const &pd = PETAL_DATA[id];
    float stack = pd.attributes.stack_damage;
    if (stack <= 0.f) return pd.damage;
    // N copies → base + stack*(N-1).
    if (sources < 1) sources = 1;
    return pd.damage + stack * (float)(sources - 1);
}

struct RotationCenter {
    float x;
    float y;
    float r;
};

static float _owner_level_mult(std::array<uint32_t, PetalID::kNumPetals> const *levels,
                               PetalID::T id) {
    if (levels == nullptr || id == PetalID::kNone || id >= PetalID::kNumPetals)
        return 1.f;
    return Craft::level_stat_mult(Craft::petal_level_get(*levels, id));
}

// Only the earliest primary slot holding this Unique PetalID receives level stats.
static uint8_t _slot_is_leveled_copy(Entity const &player, uint32_t slot_i, PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    if (!Craft::can_level_petal(id)) return 0;
    if (slot_i >= player.get_loadout_count()) return 0;
    for (uint32_t i = 0; i < slot_i; ++i) {
        if (player.loadout[i].get_petal_id() == id) return 0;
    }
    return 1;
}

static struct PlayerBuffs _get_petal_passive_buffs(
    Simulation *sim, Entity &player,
    std::array<uint32_t, PetalID::kNumPetals> const *petal_levels) {
    struct PlayerBuffs buffs = {0};
    if (player.has_component(kMob)) return buffs;
    player.set_equip_flags(0);
    player.damage_reflection = 0;
    player.poison_armor = 0;
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot const &slot = player.loadout[i];
        PetalID::T slot_petal_id = slot.get_petal_id();
        if (slot_petal_id == PetalID::kNone || slot_petal_id >= PetalID::kNumPetals)
            continue;
        // Mimic passives come from the resolved target (Magnet FOV etc.).
        PetalID::T eff_id = slot_petal_id;
        if (PETAL_DATA[slot_petal_id].attributes.mimic_left) {
            PetalID::T tgt = effective_petal_id(player, i);
            if (tgt == PetalID::kNone) continue; // inert Mimic
            eff_id = tgt;
        }
        struct PetalData const &petal_data = PETAL_DATA[eff_id];
        struct PetalAttributes const &attrs = petal_data.attributes;
        float lv_mult = _slot_is_leveled_copy(player, i, slot_petal_id)
            ? _owner_level_mult(petal_levels, slot_petal_id) : 1.f;
        if (attrs.equipment != EquipmentFlags::kNone)
            player.set_equip_flags(player.get_equip_flags() | (1 << attrs.equipment));
        float vision = Craft::level_scale_vision(attrs.vision_factor, lv_mult);
        buffs.vision_factor = std::min(buffs.vision_factor, vision);
        // Speed boosts: strongest single factor wins. Multiplying several
        // Accelerators (1.3^n) turned map travel into an accidental rocket.
        // Unique Lv scales the excess above 1 (Accelerator etc. were ignoring Lv).
        float move = Craft::level_scale_factor(attrs.move_speed_factor, lv_mult);
        if (petal_name_is(eff_id, "Mysterious Powder")) {
            bool blocked = player.powder_suppressed_ticks > 0;
            sim->spatial_hash.query(player.get_x(), player.get_y(), 240.f, 240.f,
                [&](Simulation *s, Entity &m) {
                    if (!m.has_component(kMob) || !m.has_component(kHealth)
                        || m.get_team() == player.get_team()) return;
                    if (m.get_rarity() <= attrs.powder_max_rarity) return;
                    Vector delta(m.get_x() - player.get_x(), m.get_y() - player.get_y());
                    if (delta.magnitude() <= 240.f) blocked = true;
                });
            if (blocked) move = 1.f;
        }
        if (move > 1.f)
            buffs.move_speed_factor = std::fmax(buffs.move_speed_factor, move);
        else if (move > 0.f && move < 1.f)
            buffs.move_speed_factor *= move;
        buffs.magnet_radius = std::fmax(buffs.magnet_radius, attrs.magnet_radius * lv_mult);
        if (attrs.attraction_radius > 0.f)
            buffs.attraction_radius += attrs.attraction_radius * lv_mult;
        // Air: strongest copy only (no stack). Soil / others still sum.
        if (attrs.body_radius_bonus > 0.f) {
            if (petal_name_is(eff_id, "Air"))
                buffs.air_body_radius = std::fmax(buffs.air_body_radius, attrs.body_radius_bonus);
            else
                buffs.body_radius_bonus += attrs.body_radius_bonus;
        }
        if (attrs.heal_amp > 0.f)
            buffs.heal_amp = std::fmax(buffs.heal_amp, attrs.heal_amp);
        if (attrs.luck > 0.f)
            buffs.luck += attrs.luck;
        if (attrs.damage_absorb > 0.f)
            buffs.damage_absorb += attrs.damage_absorb * lv_mult;
        if (attrs.flower_evasion > 0.f)
            buffs.flower_evasion = std::fmin(0.75f, buffs.flower_evasion + attrs.flower_evasion);
        if (attrs.nazar_charges > 0) {
            buffs.nazar_charges = attrs.nazar_charges;
            buffs.nazar_mitigation = attrs.nazar_mitigation;
        }
        if (attrs.mana_gen > 0.f)
            buffs.mana_gen += attrs.mana_gen;
        if (attrs.mana_cap > 0.f)
            buffs.mana_cap += attrs.mana_cap;
        if (attrs.petal_hp_factor > 1.f)
            buffs.petal_hp_factor = std::fmax(buffs.petal_hp_factor, attrs.petal_hp_factor);
        if (attrs.aggro_shrink > 0.f)
            buffs.aggro_shrink = std::fmax(buffs.aggro_shrink, attrs.aggro_shrink);
        if (attrs.totem_heal > 0.f) {
            buffs.totem_heal = std::fmax(buffs.totem_heal, attrs.totem_heal * lv_mult);
            buffs.totem_seconds = std::fmax(buffs.totem_seconds, attrs.totem_seconds);
        }
        if (attrs.root_armor > 0.f) {
            buffs.root_armor = std::fmax(buffs.root_armor, attrs.root_armor * lv_mult);
            buffs.root_interval = attrs.root_interval > 0.f ? attrs.root_interval : 2.f;
            if (attrs.root_max_stacks > buffs.root_max_stacks)
                buffs.root_max_stacks = attrs.root_max_stacks;
        }
        if (attrs.ankh_mark) buffs.ankh_mark = 1;
        if (attrs.shovel_burrow) {
            buffs.shovel_burrow = 1;
            buffs.shovel_reload = std::fmax(2.f, petal_data.reload);
        }
        if (attrs.wax_block) buffs.wax_block = 1;
        if (attrs.overheal_to_shield > buffs.overheal_to_shield)
            buffs.overheal_to_shield = attrs.overheal_to_shield;
        if (attrs.compass) buffs.compass = 1;
        if (attrs.dark_mark) buffs.dark_mark = 1;
        if (attrs.champion_crown) buffs.champion_crown = 1;
        if (attrs.soul_splitter) buffs.soul_splitter = 1;
        if (attrs.paranormal) buffs.paranormal = 1;
        if (petal_name_is(eff_id, "Fragment")) buffs.has_fragment = 1;
        if (petal_name_is(eff_id, "Cogwheel")) buffs.has_cogwheel = 1;
        buffs.extra_range = std::fmax(attrs.extra_range * lv_mult, buffs.extra_range);
        buffs.extra_damage = std::fmax(buffs.extra_damage, attrs.extra_body_damage * lv_mult);
        // Damage amps: max within the same petal name (no Skyblade^n), multiply
        // across different names. Final product soft-capped after the loop.
        {
            float amp = Craft::level_scale_factor(attrs.extra_damage_factor, lv_mult);
            if (amp > 1.f) {
                char const *nm = petal_data.name;
                uint32_t h = 2166136261u;
                for (char const *p = nm; *p; ++p) {
                    h ^= (uint8_t)*p;
                    h *= 16777619u;
                }
                uint32_t found = buffs.amp_name_n;
                for (uint32_t ai = 0; ai < buffs.amp_name_n; ++ai) {
                    if (buffs.amp_name_hash[ai] == h) { found = ai; break; }
                }
                if (found < buffs.amp_name_n) {
                    if (amp > buffs.amp_name_best[found]) buffs.amp_name_best[found] = amp;
                } else if (buffs.amp_name_n < PlayerBuffs::kAmpNameCap) {
                    buffs.amp_name_hash[buffs.amp_name_n] = h;
                    buffs.amp_name_best[buffs.amp_name_n] = amp;
                    ++buffs.amp_name_n;
                }
            } else if (amp > 0.f && amp < 1.f)
                buffs.damage_factor *= amp;
        }
        buffs.reload_factor *= Craft::level_scale_factor(attrs.extra_reload_factor, lv_mult);
        // Name match — ladder rarities (Common…Unique) must not miss the base ID.
        if (petal_name_is(eff_id, "Yin Yang"))
            ++buffs.yinyang_count;
        if (!player.loadout[i].already_spawned) continue;
        // constant_heal is data-driven (HP/s). Yucca keeps classic defend-only gating;
        // all other petals with the attr heal while the slot is spawned.
        if (attrs.constant_heal > 0.f) {
            // Wiki heal√3 left Ultra+ far behind combat×3 / cactus HP — catch up.
            float h = attrs.constant_heal * lv_mult * heal_tier_catchup(petal_data.rarity);
            if (petal_name_is(eff_id, "Yucca")) {
                if (BitMath::at(player.input, InputFlags::kDefending)
                    && !BitMath::at(player.input, InputFlags::kAttacking))
                    buffs.heal += h / TPS;
            } else {
                buffs.heal += h / TPS;
            }
        }
        if (attrs.petal_repair > 0.f)
            buffs.petal_repair += attrs.petal_repair * lv_mult
                * heal_tier_catchup(petal_data.rarity);
        buffs.extra_rot += attrs.extra_rotation_speed * lv_mult;
        if (attrs.extra_health > 0.f && attrs.extra_health < 2.f
            && petal_name_is(eff_id, "Mysterious Relic")) {
            if (attrs.extra_health > buffs.relic_hp_frac)
                buffs.relic_hp_frac = attrs.extra_health;
        } else {
            buffs.extra_health += attrs.extra_health * lv_mult;
        }
        // Corruption cannot be Mimicked (resolve already rejects); keep check on eff.
        if (attrs.corruption) {
            buffs.has_corruption = 1;
            if (petal_data.rarity >= RarityID::kUnique)
                buffs.unique_corruption = 1;
        }
        player.damage_reflection = std::fmax(player.damage_reflection, attrs.damage_reflection);
        player.poison_armor = std::fmax(player.poison_armor, attrs.poison_armor * lv_mult / TPS);
        // Poison-body Cactus is a single ID (name shares "Cactus" with HP cactus).
        if (eff_id == PetalID::kPoisonCactus)
            buffs.is_poisonous = 1;
        // King's Oath: low-HP damage surge (on top of petal extra_damage_factor)
        if (petal_name_is(eff_id, "King's Oath")
            && player.max_health > 0.f
            && player.health / player.max_health < 0.35f)
            buffs.damage_factor *= 1.35f;
        // Royal Banner / Phantom Eye spin & damage come only from PETAL_DATA attrs
        // (no extra hardcoded stacks — keeps tooltip values honest).
    }
    {
        float prod = 1.f;
        for (uint32_t ai = 0; ai < buffs.amp_name_n; ++ai)
            prod *= buffs.amp_name_best[ai];
        // Soft ceiling — same-name amps already take max (no Skyblade^n).
        // 5× made Unique multi-amp loadouts feel "damage capped" vs Ultra+.
        if (prod > 8.f) prod = 8.f;
        if (prod > 1.f) buffs.damage_factor *= prod;
    }
    return buffs;
}

static uint32_t _get_petal_rotation_count(Simulation *sim, Entity &player) {
    uint32_t count = 0;
    for (uint8_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot const &slot = player.loadout[i];
        auto effective = effective_petal_id(player, i);
        if (effective == PetalID::kNone) continue;
        struct PetalData const &petal_data = PETAL_DATA[effective];
        if (petal_data.attributes.clump_radius > 0)
            ++count;
        else {
            for (uint32_t j = 0; j < slot.size(); ++j) {
                if (!sim->ent_alive(slot.petals[j].ent_id))
                    ++count;
                else if (!BitMath::at(sim->get_ent(slot.petals[j].ent_id).flags, EntityFlags::kIsDetached))
                    ++count;
            }
        }
    }
    return count;
}

// gardn-master: if a Moon is out, every other petal orbits the moon entity.
static RotationCenter const _get_petal_rotation_center(Simulation *sim, Entity const &player) {
    RotationCenter rotation_center = { 
        .x = player.get_x(), 
        .y = player.get_y(), 
        .r = player.get_radius() 
    };
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot const &slot = player.loadout[i];
        if (!petal_is_moon(effective_petal_id(player, i))) continue;
        for (uint32_t j = 0; j < slot.size(); ++j) {
            LoadoutPetal const &petal_slot = slot.petals[j];
            if (!sim->ent_alive(petal_slot.ent_id)) continue;
            Entity &moon = sim->get_ent(petal_slot.ent_id);
            // Must stay detached or Flower will treat it like a normal ring petal.
            BitMath::set(moon.flags, EntityFlags::kIsDetached);
            rotation_center.x = moon.get_x();
            rotation_center.y = moon.get_y();
            rotation_center.r = moon.get_radius();
            return rotation_center;
        }
    }
    return rotation_center;
}

void tick_player_behavior(Simulation *sim, Entity &player) {
    if (player.pending_delete) return;
    // Party revive corpses are kFlower without kHealth — visual only.
    if (!player.has_component(kHealth)) return;
    DEBUG_ONLY(assert(player.max_health > 0);)
    float talent_hp = 0;
    float talent_rot = 0;
    float talent_reload = 1.f;
    float talent_petal_hp = 1.f;
    float rot_mult = Talent::rot_setting_mult(Talent::kRotSettingDefault);
    std::array<uint32_t, PetalID::kNumPetals> const *petal_levels = nullptr;
    if (!player.has_component(kMob)) {
        if (Client *owner = Server::client_by_camera(sim, player.get_parent())) {
            petal_levels = &owner->petal_levels;
            uint8_t const *r = owner->talent_ranks;
            talent_hp = Talent::health_bonus(r[TalentID::kHealth]);
            talent_rot = Talent::rot_bonus(r[TalentID::kRotSpeed]);
            talent_reload = Talent::reload_mult(r[TalentID::kCooldown]);
            talent_petal_hp = Talent::petal_health_mult(r[TalentID::kPetalArmor]);
            rot_mult = Talent::rot_setting_mult(owner->rot_speed_setting);
        }
    }
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        auto &slot = player.loadout[i];
        auto requested = player.get_loadout_ids(i);
        auto effective = effective_petal_id(player, i);
        if (slot.get_petal_id() != requested || slot.behavior_id != effective) {
            slot.update_id(sim, requested);
            slot.behavior_id = effective;
            // Battery / Nazar / Wax: start fully charged so equip ≠ dead petal.
            if (effective != PetalID::kNone && effective < PetalID::kNumPetals) {
                auto const &na = PETAL_DATA[effective].attributes;
                uint8_t cap = na.wax_block ? 1
                    : (na.nazar_charges ? na.nazar_charges : na.battery_charges);
                if (cap) slot.effect_charges = cap;
            }
        }
        if (effective == PetalID::kNone) continue;
        auto const &pd = PETAL_DATA[effective];
        auto const &a = pd.attributes;
        uint8_t capacity = a.wax_block ? 1 : (a.nazar_charges ? a.nazar_charges : a.battery_charges);
        if (capacity && slot.effect_charges < capacity) {
            if (++slot.effect_reload >= std::fmax(2.f, pd.reload * TPS)) {
                slot.effect_charges = capacity;
                slot.effect_reload = 0;
            }
        }
    }
    PlayerBuffs buffs = _get_petal_passive_buffs(sim, player, petal_levels);
    if (!player.has_component(kMob)) {
        if (Client *owner = Server::client_by_camera(sim, player.get_parent())) {
            uint8_t const *r = owner->talent_ranks;
            buffs.extra_range += Talent::range_bonus(r[TalentID::kAttackRange]);
            buffs.magnet_radius += Talent::magnet_bonus(r[TalentID::kMagnet]);
        }
    }
    buffs.reload_factor *= talent_reload;
    // Golden Leaf CDR cap (99%): stacked leaves + Unique Lv + talent cannot
    // push below kReloadFactorFloor (≤0 deleted petal art; ≥2-tick floor below).
    if (buffs.reload_factor < Craft::kReloadFactorFloor)
        buffs.reload_factor = Craft::kReloadFactorFloor;
    if (buffs.reload_factor > Craft::kReloadFactorCeil)
        buffs.reload_factor = Craft::kReloadFactorCeil;
    double health_ratio = player.health / player.max_health;
    if (!player.has_component(kMob)) {
        uint32_t const sc = player.get_score();
        uint32_t level = score_to_level(sc);
        float base_hp = hp_at_level(level);
        player.max_health = base_hp * (1.f + buffs.relic_hp_frac)
            + buffs.extra_health + talent_hp;
        player.damage = body_damage_at_level(level) + buffs.extra_damage;
        // Keep account/pending XP high-water every tick so refresh cannot miss
        // mid-level progress if disconnect capture races the flower teardown.
        if (Client *owner = Server::client_by_camera(sim, player.get_parent()))
            AccountStore::note_score(owner, sc);
    }
    player.health = health_ratio * player.max_health;
    // Air / Soil: body size (cap so spatial hash stays valid)
    if (!player.has_component(kMob)) {
        float br = BASE_FLOWER_RADIUS + buffs.body_radius_bonus + buffs.air_body_radius;
        if (br < 12.f) br = 12.f;
        if (br > 70.f) br = 70.f;
        player.set_radius(br);
    }
    if (buffs.heal > 0)
        inflict_heal(sim, player, buffs.heal);
    // Salve: restore durability on living loadout petals (not the Salve itself).
    if (buffs.petal_repair > 0.f) {
        float const per_tick = buffs.petal_repair / (float) TPS;
        for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
            LoadoutSlot const &slot = player.loadout[i];
            PetalID::T pid = slot.get_petal_id();
            if (pid == PetalID::kNone || pid >= PetalID::kNumPetals) continue;
            if (petal_name_is(pid, "Salve")) continue;
            for (uint32_t j = 0; j < slot.size(); ++j) {
                EntityID eid = slot.petals[j].ent_id;
                if (!sim->ent_alive(eid)) continue;
                Entity &petal = sim->get_ent(eid);
                if (!petal.has_component(kHealth) || !petal.has_component(kPetal)) continue;
                if (petal.health >= petal.max_health) continue;
                petal.health = std::fmin(petal.max_health, petal.health + per_tick);
                if (petal.max_health > 0.f)
                    petal.set_health_ratio(petal.health / petal.max_health);
            }
        }
    }
    if (buffs.is_poisonous)
        player.poison_damage = {10.0, 2};
    else
        player.poison_damage = {0, 0};

    // Mana, temporary armor and defensive effects use explicit lifetimes.
    if (!player.has_component(kMob)) {
        if (player.powder_suppressed_ticks) --player.powder_suppressed_ticks;
        player.armor = std::fmax(0.f, player.armor - player.root_armor_applied);
        player.root_armor_applied = 0.f;
        if (buffs.has_fragment)
            buffs.damage_factor *= 1.03f;
        if (buffs.has_cogwheel)
            buffs.reload_factor *= 0.97f;
        // Mana Orb / Magic Leaf+Cactus capacity + gen
        float mana_cap = buffs.mana_cap;
        if (mana_cap > 0.f) {
            if (buffs.mana_gen > 0.f)
                player.mana += buffs.mana_gen / (float) TPS;
            if (player.mana > mana_cap) player.mana = mana_cap;
            if (player.mana < 0.f) player.mana = 0.f;
        } else if (buffs.mana_gen <= 0.f) {
            player.mana = 0.f;
        }
        // Root: stack armor every interval up to max stacks
        if (buffs.root_armor > 0.f && buffs.root_max_stacks > 0) {
            game_tick_t need = (game_tick_t)(buffs.root_interval * TPS);
            if (need < 1) need = 1;
            ++player.root_tick;
            if (player.root_tick >= need) {
                player.root_tick = 0;
                if (player.root_stacks < buffs.root_max_stacks)
                    ++player.root_stacks;
            }
            player.root_armor_applied = buffs.root_armor * player.root_stacks;
            player.armor += player.root_armor_applied;
        } else {
            player.root_stacks = 0;
            player.root_tick = 0;
        }
        // Totem on-kill heal buff
        player.totem_heal_rate = buffs.totem_heal;
        if (buffs.totem_heal <= 0.f) player.totem_ticks = 0;
        if (player.totem_ticks > 0) {
            --player.totem_ticks;
            if (player.totem_heal_rate > 0.f)
                inflict_heal(sim, player, player.totem_heal_rate / (float) TPS);
        }
        player.overheal_shield_ratio = buffs.overheal_to_shield;
        // Shovel activates once per defend press, with a finite burrow and cooldown.
        if (player.burrow_cooldown) --player.burrow_cooldown;
        if (player.burrow_ticks) --player.burrow_ticks;
        if (buffs.shovel_burrow && !player.burrow_cooldown
            && BitMath::at(player.input, InputFlags::kDefending)
            && !BitMath::at(player.previous_input, InputFlags::kDefending)) {
            player.burrow_ticks = (game_tick_t)(0.6f * TPS);
            player.burrow_cooldown = (game_tick_t)(buffs.shovel_reload * TPS);
            player.immunity_ticks = std::max(player.immunity_ticks, player.burrow_ticks);
        }
        player.previous_input = player.input;
        // Sponge's first hit fixes the settlement deadline; subsequent hits
        // cannot keep rolling it forward. Raw settlement still runs lethal saves.
        if (player.pending_damage_ticks && --player.pending_damage_ticks == 0) {
            float due = player.pending_damage;
            auto dealer = player.pending_damage_dealer;
            player.pending_damage = 0.f;
            player.pending_damage_dealer = NULL_ENTITY;
            if (due > 0.f) inflict_damage(sim, dealer, player.id, due, DamageType::kDeferred);
        }
        // Champion crown: small aura damage while attacking
        if (buffs.champion_crown
            && BitMath::at(player.input, InputFlags::kAttacking)) {
            float const R = 120.f;
            float const dmg = std::fmax(8.f, player.damage * 0.15f);
            float const px = player.get_x(), py = player.get_y();
            EntityID team = player.get_team();
            EntityID src = player.id;
            sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
                if (!ent.has_component(kHealth) || !ent.has_component(kMob)) return;
                if (ent.get_team() == team) return;
                float dx = ent.get_x() - px, dy = ent.get_y() - py;
                if (dx * dx + dy * dy > R * R) return;
                inflict_damage(s, src, ent.id, dmg / (float) TPS, DamageType::kSplash);
            });
            player.champion_aura = 1.f;
        } else {
            player.champion_aura = 0.f;
        }
        // Dark Mark / Compass / Paranormal / Soul Splitter flags
        player.compass_reveal = buffs.compass ? 1 : 0;
        player.paranormal_vision = buffs.paranormal ? 1 : 0;
        if (buffs.compass)
            buffs.vision_factor = std::fmin(buffs.vision_factor, 0.82f);
        if (buffs.soul_splitter
            && BitMath::at(player.input, InputFlags::kDefending)
            && player.soul_split_cd == 0) {
            player.soul_split_cd = (game_tick_t)(12 * TPS);
            Entity &mob = alloc_mob(sim, MobID::kWorkerAnt, player.get_x(), player.get_y(),
                player.get_team(), [&](Entity &m) {
                m.set_color(player.get_color());
                BitMath::set(m.flags, EntityFlags::kNoDrops);
                BitMath::set(m.flags, EntityFlags::kDieOnParentDeath);
                BitMath::set(m.flags, EntityFlags::kIsDetached);
                m.set_parent(player.id);
                m.base_entity = player.id;
                m.max_health = std::fmax(10.f, player.max_health * 0.12f);
                m.health = m.max_health;
                m.damage = std::fmax(1.f, player.damage * 0.25f);
                if (m.score_reward == 0) m.score_reward = 1;
            });
            entity_set_despawn_tick(mob, (game_tick_t)(6 * TPS));
        }
        if (player.soul_split_cd > 0) --player.soul_split_cd;
        if (!player.death_delay_ticks && player.health > player.max_health * 0.2)
            player.death_delay_used = 0;
        // Death-delay countdown (Bandage): expire → kill
        if (player.death_delay_ticks > 0) {
            --player.death_delay_ticks;
            if (player.death_delay_ticks == 0 && player.health <= 1.f)
                player.health = 0.f;
        }
    }

    // Corruption: red body + Unique spreads temporary corruption to nearby flowers.
    if (!player.has_component(kMob)) {
        if (player.corruption_ticks > 0) --player.corruption_ticks;
        if (buffs.unique_corruption) {
            // Was 160 (≈ petal orbit) — victims had to stand on top of the
            // Unique wearer to turn red. Match Magnet-scale presence (~screen).
            float const R = 600.f;
            float const px = player.get_x();
            float const py = player.get_y();
            sim->spatial_hash.query(px, py, R, R, [&](Simulation *s, Entity &ent) {
                if (!ent.has_component(kFlower) || ent.has_component(kMob)) return;
                if (ent.id == player.id) return;
                if (!ent.has_component(kHealth)) return;
                float dx = ent.get_x() - px;
                float dy = ent.get_y() - py;
                if (dx * dx + dy * dy > R * R) return;
                // Hold red ~3s after leaving the edge so the aura reads clearly.
                if (ent.corruption_ticks < 3 * TPS) ent.corruption_ticks = 3 * TPS;
            });
        }
        if (buffs.has_corruption || player.corruption_ticks > 0)
            player.set_color(ColorID::kRed);
        else if (player.get_color() == ColorID::kRed)
            player.set_color(ColorID::kYellow);
    }

    uint8_t effects = 0;
    if (player.burrow_ticks) effects |= FlowerEffects::kBurrow;
    if (player.death_delay_ticks) effects |= FlowerEffects::kDeathGrace;
    if (buffs.mana_cap > 0.f) effects |= FlowerEffects::kMana;
    if (buffs.aggro_shrink > 0.f) effects |= FlowerEffects::kLowAggro;
    if (buffs.shovel_burrow) effects |= FlowerEffects::kShovel;
    player.set_effect_flags(effects);
    player.set_mana_ratio(buffs.mana_cap > 0.f ? player.mana / buffs.mana_cap : 0.f);
    player.set_shield_ratio(player.max_health > 0.f ? player.flower_shield / player.max_health : 0.f);
    player.set_ability_cooldown(buffs.shovel_burrow ? player.burrow_cooldown : 0);
    float rot_pos = 0;
    uint32_t rotation_count = _get_petal_rotation_count(sim, player);
    RotationCenter const rotation_center = _get_petal_rotation_center(sim, player);
    //maybe use delta mode for face flags?
    player.set_face_flags(0);

    if (sim->ent_alive(player.get_parent())) {
        Entity &camera = sim->get_ent(player.get_parent());
        camera.set_fov(BASE_FOV * buffs.vision_factor);
    }
    if (buffs.move_speed_factor != 1.f &&
        (player.acceleration.x != 0.f || player.acceleration.y != 0.f))
        player.acceleration *= buffs.move_speed_factor;
    // Magnet (二创): proximity scoop + soft pull toward flower. No speed boost.
    {
        float pickup_r = player.get_radius() + 30.f + 12.f;
        float pickup_r2 = pickup_r * pickup_r;
        float magnet_r = buffs.magnet_radius;
        float magnet_r2 = magnet_r > 0.f ? magnet_r * magnet_r : 0.f;
        float query_r = magnet_r > pickup_r ? magnet_r : pickup_r;
        sim->spatial_hash.query(
            player.get_x(), player.get_y(), query_r, query_r,
            [&](Simulation *, Entity &drop) {
            if (!drop.has_component(kDrop) || drop.pending_delete) return;
            if (!Loot::can_access_drop(drop, player.get_parent()) || drop.immunity_ticks > 0) return;
            float dx = player.get_x() - drop.get_x();
            float dy = player.get_y() - drop.get_y();
            float d2 = dx * dx + dy * dy;
            if (d2 <= pickup_r2) {
                drop.velocity.set(0, 0);
                drop.acceleration.set(0, 0);
                drop.collision_velocity.set(0, 0);
                pickup_drop(sim, player, drop);
                return;
            }
            if (magnet_r2 <= 0.f || d2 > magnet_r2) return;
            if (d2 <= 1.f) return;
            float dist = std::sqrt(d2);
            float t = dist / magnet_r;
            float pull = PLAYER_ACCELERATION * (3.f + 5.f * t);
            drop.acceleration.set(dx / dist * pull, dy / dist * pull);
            float vx = drop.velocity.x;
            float vy = drop.velocity.y;
            float radial = (vx * dx + vy * dy) / dist;
            float tx = vx - dx / dist * radial;
            float ty = vy - dy / dist * radial;
            drop.velocity.x -= tx * 0.35f;
            drop.velocity.y -= ty * 0.35f;
            // Let Motion skip terrain so berms/walls don't block the suck.
            BitMath::set(drop.flags, EntityFlags::kMagnetPull);
        });
    }

    // Stick / Magic Stick sandstorms: hard cap + cull. 4×Unique used to paint
    // 100+ giant storms and crash phones (radius×2 Unique art is expensive).
    constexpr uint32_t kStickSummonCap = 6;
    uint32_t stick_summons = 0;
    uint8_t stick_counted = 0;

    DEBUG_ONLY(assert(player.get_loadout_count() <= MAX_SLOT_COUNT);)
    for (uint32_t i = 0; i < player.get_loadout_count(); ++i) {
        LoadoutSlot &slot = player.loadout[i];
        //player.set_loadout_ids(i, slot.id);
        //other way around. loadout_ids should dictate loadout
        if (slot.get_petal_id() != player.get_loadout_ids(i))
            slot.update_id(sim, player.get_loadout_ids(i));
        PetalID::T slot_petal_id = slot.get_petal_id();
        if (slot_petal_id == PetalID::kNone || slot_petal_id >= PetalID::kNumPetals)
            continue;
        struct PetalData const &slot_data = PETAL_DATA[slot_petal_id];
        // Mimic behaves as the left petal at Mimic's rarity (wiki). Spawn that
        // ID so Peas/Eggs/poison/etc. actually run — old code kept Mimic id.
        PetalID::T behave_id = slot_petal_id;
        uint8_t mimic_inert = 0;
        if (slot_data.attributes.mimic_left) {
            PetalID::T tgt = effective_petal_id(player, i);
            if (tgt == PetalID::kNone) mimic_inert = 1;
            else behave_id = tgt;
        }
        struct PetalData const &petal_data = PETAL_DATA[behave_id];
        DEBUG_ONLY(assert(petal_data.count <= MAX_PETALS_IN_CLUMP);)

        if (slot_data.count == 0)
            continue;
        if (mimic_inert) {
            for (uint32_t j = 0; j < slot.size(); ++j) {
                LoadoutPetal &petal_slot = slot.petals[j];
                if (sim->ent_alive(petal_slot.ent_id))
                    sim->request_delete(petal_slot.ent_id);
                petal_slot.ent_id = NULL_ENTITY;
                petal_slot.reload = 0;
            }
            slot.already_spawned = 0;
            player.set_loadout_reloads(i, 0);
            player.set_loadout_healths(i, 0);
            continue;
        }
        // Body-mounted equipment (Cutter / Accelerator / Magnet / …): passive +
        // flower art only — never spawn an orbiting clone.
        if (petal_data.attributes.equipment != EquipmentFlags::kNone) {
            for (uint32_t j = 0; j < slot.size(); ++j) {
                LoadoutPetal &petal_slot = slot.petals[j];
                if (sim->ent_alive(petal_slot.ent_id))
                    sim->request_delete(petal_slot.ent_id);
                petal_slot.ent_id = NULL_ENTITY;
                petal_slot.reload = 0;
            }
            slot.already_spawned = 1;
            player.set_loadout_reloads(i, 255);
            player.set_loadout_healths(i, 255);
            continue;
        }
        float min_reload = 1;
        float health_sum = 0.f;
        uint32_t health_n = 0;
        for (uint32_t j = 0; j < slot.size(); ++j) {
            LoadoutPetal &petal_slot = slot.petals[j];
            if (!sim->ent_alive(petal_slot.ent_id)) {
                petal_slot.ent_id = NULL_ENTITY;
                float reload_f = petal_data.reload * (float) TPS * buffs.reload_factor;
                if (reload_f < 2.f) reload_f = 2.f; // ≥2 ticks so art can render
                game_tick_t reload_time = (game_tick_t) reload_f;
                if (!slot.already_spawned) reload_time += TPS;
                float this_reload = (float) petal_slot.reload / (float) reload_time;
                min_reload = std::min(min_reload, this_reload);
                if (petal_slot.reload >= reload_time) {
                    // Petals with mana_spawn_cost and no mob summon: gate entity spawn on mana.
                    float spawn_cost = petal_data.attributes.mana_spawn_cost;
                    if (spawn_cost > 0.f && petal_data.attributes.spawns == MobID::kNumMobs) {
                        if (player.mana < spawn_cost)
                            continue; // stay at ready reload until mana available
                        player.mana -= spawn_cost;
                    }
                    petal_slot.ent_id = alloc_petal(sim, behave_id, player).id;
                    Entity &spawned = sim->get_ent(petal_slot.ent_id);
                    // Card: small random hp/dmg variance at spawn
                    if (petal_data.attributes.card_random_stats) {
                        float f = 0.7f + frand() * 0.6f; // 0.7–1.3
                        spawned.damage *= f;
                        float hf = 0.7f + frand() * 0.6f;
                        spawned.max_health *= hf;
                        spawned.health = spawned.max_health;
                    }
                    // Triangle: base only for 1 copy; each extra same-rarity adds stack.
                    if (petal_data.attributes.stack_damage > 0.f)
                        spawned.damage = _triangle_base_damage(
                            behave_id, _triangle_stack_sources(player, behave_id));
                    spawned.damage *= buffs.damage_factor;
                    float hp_mult = talent_petal_hp * buffs.petal_hp_factor;
                    // Unique level: scale from the Mimic slot's own Unique Lv.
                    float lv_mult = _slot_is_leveled_copy(player, i, slot_petal_id)
                        ? _owner_level_mult(petal_levels, slot_petal_id) : 1.f;
                    if (lv_mult != 1.f) {
                        spawned.damage *= lv_mult;
                        spawned.poison_damage.damage *= lv_mult;
                        spawned.burn_damage.damage *= lv_mult;
                        spawned.armor *= lv_mult;
                        hp_mult *= lv_mult;
                    }
                    // Lightning body damage is 0 in StaticData — bake flash into
                    // petal.damage so contact + Unique Lv / amp actually land.
                    if (petal_data.damage <= 0.f
                        && petal_data.attributes.lightning_damage > 0.f
                        && petal_data.attributes.battery_charges == 0) {
                        float flash = petal_data.attributes.lightning_damage
                            * buffs.damage_factor;
                        if (lv_mult != 1.f) flash *= lv_mult;
                        spawned.damage = flash;
                    }
                    if (hp_mult != 1.f) {
                        spawned.max_health *= hp_mult;
                        spawned.health = spawned.max_health;
                    }
                    spawned.set_x(rotation_center.x);
                    spawned.set_y(rotation_center.y);
                    petal_slot.reload = 0;
                    slot.already_spawned = 1;
                } 
                else
                    ++petal_slot.reload;
            }
            if (sim->ent_alive(petal_slot.ent_id)) {
                Entity &petal = sim->get_ent(petal_slot.ent_id);
                // Keep Triangle stack in sync when the player adds/removes copies
                // mid-life (spawn-only calc left the first petal stuck at 1-stack).
                if (petal_data.attributes.stack_damage > 0.f
                    && !BitMath::at(petal.flags, EntityFlags::kIsDespawning)) {
                    float dmg = _triangle_base_damage(
                        behave_id, _triangle_stack_sources(player, behave_id));
                    dmg *= buffs.damage_factor;
                    float lv_mult = _slot_is_leveled_copy(player, i, slot_petal_id)
                        ? _owner_level_mult(petal_levels, slot_petal_id) : 1.f;
                    if (lv_mult != 1.f) dmg *= lv_mult;
                    petal.damage = dmg;
                }
                // Tomato / Clay: charge damage toward *_max_damage over ~5s alive
                float charge_max = petal_data.attributes.tomato_max_damage > 0.f
                    ? petal_data.attributes.tomato_max_damage
                    : petal_data.attributes.clay_max_damage;
                if (charge_max > 0.f
                    && !BitMath::at(petal.flags, EntityFlags::kIsDespawning)) {
                    float base = petal_data.damage * buffs.damage_factor;
                    float lv_mult = _slot_is_leveled_copy(player, i, slot_petal_id)
                        ? _owner_level_mult(petal_levels, slot_petal_id) : 1.f;
                    if (lv_mult != 1.f) base *= lv_mult;
                    float maxd = charge_max * buffs.damage_factor;
                    if (lv_mult != 1.f) maxd *= lv_mult;
                    float t = (float) petal.lifetime / (float)(5 * TPS);
                    if (t > 1.f) t = 1.f;
                    petal.damage = base + (maxd - base) * t;
                    if (petal_data.attributes.clay_max_damage > 0.f) {
                        float br = petal_data.radius;
                        petal.set_radius(br * (1.f + 0.75f * t));
                    }
                }
                if (petal.max_health > 0.f) {
                    float hr = petal.health / petal.max_health;
                    if (hr < 0.f) hr = 0.f;
                    if (hr > 1.f) hr = 1.f;
                    health_sum += hr;
                    ++health_n;
                }
                //only do this if petal not despawning
                if (petal.has_component(kPetal) && 
                    !BitMath::at(petal.flags, EntityFlags::kIsDespawning) &&
                    !BitMath::at(petal.flags, EntityFlags::kIsDetached)) {
                    // Rose / burst-heal seeking the flower: skip orbit so a still
                    // player does not keep yanking the petal back to the ring.
                    game_tick_t const sec_reload_ticks =
                        (game_tick_t)(petal_data.attributes.secondary_reload * TPS);
                    uint8_t const seeking_shell =
                        petal_name_is(behave_id, "Shell")
                        && petal_data.attributes.burst_heal > 0;
                    uint8_t const seeking_burst_heal =
                        petal_data.attributes.burst_heal > 0
                        && petal_data.attributes.secondary_reload > 0
                        && petal.secondary_reload > sec_reload_ticks
                        && player.dandy_ticks == 0
                        && (seeking_shell || player.health < player.max_health);
                    if (!seeking_burst_heal) {
                    //petal rotation behavior
                    Vector wanting;
                    Vector delta(rotation_center.x - petal.get_x(), rotation_center.y - petal.get_y());
                    if (rotation_count > 0)
                        wanting.unit_normal(2 * M_PI * rot_pos / rotation_count + player.heading_angle);

                    float range = rotation_center.r + PETAL_ORBIT_IDLE;
                    uint8_t const attacking =
                        BitMath::at(player.input, InputFlags::kAttacking);
                    if (attacking) {
                        if (petal_data.attributes.defend_only == 0) {
                            // Sawblade ignores Reach / Third Eye orbit extension.
                            float extra = petal_data.attributes.fixed_orbit_range
                                ? 0.f : buffs.extra_range;
                            range = rotation_center.r + PETAL_ORBIT_ATTACK + extra;
                        }
                        if (petal_name_is(petal.get_petal_id(), "Wing")) {
                            float wave = sinf((float) petal.lifetime / (0.4 * TPS));
                            wave = wave * wave;
                            range += wave * PETAL_WING_WAVE_EXTRA;
                        }
                    }
                    else if (BitMath::at(player.input, InputFlags::kDefending)) 
                        range = rotation_center.r + PETAL_ORBIT_DEFEND;
                    wanting *= range;
                    if (petal_data.attributes.clump_radius > 0) {
                        Vector secondary;
                        secondary.unit_normal(2 * M_PI * j / petal_data.count + player.heading_angle * 0.2)
                        .set_magnitude(petal_data.attributes.clump_radius);
                        wanting += secondary;
                    }
                    wanting += delta;
                    // High rot cuts the corner of the orbit (加速追不上角速度 → 半径塌缩).
                    // Seek harder as rot_mult rises so attack range stays near PETAL_ORBIT_ATTACK.
                    float seek = 0.5f + 0.4f * fminf(1.f, fmaxf(0.f, (rot_mult - 0.5f) / 1.35f));
                    if (seek > 0.92f) seek = 0.92f;
                    wanting *= seek;
                    // Extra radial restore when attacking and sitting inside the ring.
                    // Skipped when Lentil has a lock — outward restore cancelled the pull.
                    uint8_t lentil_lock = 0;
                    float lentil_tx = 0.f, lentil_ty = 0.f;
                    // Lentil: always attract (idle/defend/attack), not attack-gated.
                    if (buffs.attraction_radius > 0.f
                        && petal_data.attributes.defend_only == 0) {
                        // Reach = attack-orbit base + attraction (extra past attack ring),
                        // so idle/defend keep the same lock sphere as holding attack.
                        float attract_base = rotation_center.r + PETAL_ORBIT_ATTACK;
                        if (!petal_data.attributes.fixed_orbit_range)
                            attract_base += buffs.extra_range;
                        float const ar = attract_base + buffs.attraction_radius;
                        float best = ar;
                        float const fx = player.get_x();
                        float const fy = player.get_y();
                        sim->spatial_hash.query(fx, fy, ar, ar,
                            [&](Simulation *s, Entity &ent) {
                            if (!s->ent_alive(ent.id)) return;
                            if (ent.get_team() == player.get_team()) return;
                            if (ent.immunity_ticks > 0) return;
                            if (!ent.has_component(kMob)) return; // petals → mobs only
                            // Training dummies ARE valid Lentil locks — players
                            // equip 小扁豆 specifically to pull petals onto the ring.
                            if (is_player_faction(s, ent)) return;
                            float dx = ent.get_x() - fx;
                            float dy = ent.get_y() - fy;
                            // Count contact distance (edge), not center — fat Ultras
                            // looked "in range" but failed a center check.
                            float dist = std::sqrt(dx * dx + dy * dy) - ent.get_radius();
                            if (dist < 0.f) dist = 0.f;
                            if (dist < best) {
                                best = dist;
                                lentil_tx = ent.get_x();
                                lentil_ty = ent.get_y();
                                lentil_lock = 1;
                            }
                        });
                    }
                    if (attacking && petal_data.attributes.defend_only == 0 && !lentil_lock) {
                        float cur_r = delta.magnitude();
                        if (cur_r > 1.f && cur_r < range * 0.88f) {
                            Vector out = delta * -1.f;
                            out.set_magnitude((range - cur_r) * 0.55f);
                            wanting += out;
                        }
                    }
                    // Lentil: home almost fully toward the locked mob. Orbit seek
                    // (wanting already includes flower←petal delta) ate soft blends.
                    if (lentil_lock) {
                        float const px = petal.get_x();
                        float const py = petal.get_y();
                        Vector pull(lentil_tx - px, lentil_ty - py);
                        float d = pull.magnitude();
                        if (d > 1.f) {
                            // Half pull vs prior (吸引力减半); still closes gap.
                            float mag = d * 0.475f;
                            if (mag < 60.f) mag = 60.f;
                            if (mag > 210.f) mag = 210.f;
                            pull.set_magnitude(mag);
                            wanting = wanting * 0.525f + pull;
                        }
                    }
                    petal.acceleration = wanting;
                    }
                    if (petal_data.attributes.spawns != MobID::kNumMobs &&
                        petal.secondary_reload > sec_reload_ticks) {
                        uint8_t spawn_id = petal_data.attributes.spawns;
                        uint8_t const petal_r = petal_data.rarity;
                        // Any sandstorm-summon petal (Stick / Magic Stick): 1 per pulse, hard cap.
                        bool const is_stick = (spawn_id == MobID::kSandstorm);
                        bool const is_pharaoh = petal_name_is(behave_id, "Pharaoh's Crown");
                        bool const is_square = (spawn_id == MobID::kSquare);
                        uint8_t summon_n = 1;
                        if (is_stick) {
                            if (!stick_counted) {
                                stick_counted = 1;
                                // Collect owned storms; cull excess so a broken count can't snowball.
                                EntityID owned[64];
                                uint32_t n_owned = 0;
                                sim->for_each<kMob>([&](Simulation *s, Entity &m) {
                                    if (!s->ent_alive(m.id)) return;
                                    if (m.get_mob_id() != MobID::kSandstorm) return;
                                    // Parent or base_entity = flower (AI may re-leash parent).
                                    if (!(m.get_parent() == player.id)
                                        && !(m.base_entity == player.id)) return;
                                    if (!BitMath::at(m.flags, EntityFlags::kNoDrops)
                                        && !BitMath::at(m.flags, EntityFlags::kDieOnParentDeath))
                                        return;
                                    if (n_owned < 64) owned[n_owned++] = m.id;
                                    ++stick_summons;
                                });
                                if (stick_summons > kStickSummonCap) {
                                    uint32_t kill = stick_summons - kStickSummonCap;
                                    for (uint32_t ki = 0; ki < kill && ki < n_owned; ++ki)
                                        sim->request_delete(owned[ki]);
                                    stick_summons = kStickSummonCap;
                                }
                            }
                            if (stick_summons >= kStickSummonCap) summon_n = 0;
                        }
                        // Magic Stick: pay mana only when a summon will actually spawn.
                        float const mcost = petal_data.attributes.mana_spawn_cost;
                        if (mcost > 0.f && summon_n > 0) {
                            if (player.mana < mcost) {
                                petal.secondary_reload = 0;
                                continue;
                            }
                            player.mana -= mcost;
                        }
                        // Softened Unique end — old 196830 Unique storms melted phones + content.
                        static float const kStickSummonStat[RarityID::kNumRarities] = {
                            30.f, 90.f, 270.f, 810.f, 2430.f, 5000.f, 9000.f, 14000.f, 22000.f
                        };
                        // Pharaoh undead beetles — plain zone beetles melted vs mid content.
                        static float const kPharaohHp[RarityID::kNumRarities] = {
                            180.f, 540.f, 1620.f, 4860.f, 14580.f, 43740.f, 90000.f, 160000.f, 280000.f
                        };
                        static float const kPharaohDmg[RarityID::kNumRarities] = {
                            45.f, 135.f, 405.f, 1215.f, 3645.f, 10935.f, 22000.f, 40000.f, 72000.f
                        };
                        // Square summons — deliberately overtuned (egg→tanky square ally).
                        // Wild Unique Square stays a weak easter egg; only hatch pets use these.
                        static float const kSquareHp[RarityID::kNumRarities] = {
                            400.f, 1200.f, 3600.f, 10800.f, 32400.f, 97200.f, 200000.f, 400000.f, 800000.f
                        };
                        static float const kSquareDmg[RarityID::kNumRarities] = {
                            80.f, 240.f, 720.f, 2160.f, 6480.f, 19440.f, 45000.f, 90000.f, 180000.f
                        };
                        for (uint8_t si = 0; si < summon_n; ++si) {
                            Entity &mob = alloc_mob(sim, spawn_id, petal.get_x(), petal.get_y(), petal.get_team(), [&](Entity &mob){
                                // Always leash to the flower — egg petals die / orbit and
                                // must not be the AI parent (that parked summons in place).
                                mob.set_parent(player.id);
                                mob.set_color(player.get_color());
                                mob.base_entity = player.id;
                                BitMath::set(mob.flags, EntityFlags::kDieOnParentDeath);
                                BitMath::set(mob.flags, EntityFlags::kNoDrops);
                                BitMath::set(mob.flags, EntityFlags::kIsDetached);
                                // Keep score_reward so escort-freeze (0 dmg + 0 xp + NoDrops)
                                // cannot match hatch pets.
                                if (mob.score_reward == 0)
                                    mob.score_reward = 1;
                                // Match egg petal rarity (zone spawns already do this).
                                uint8_t base_r = MOB_DATA[spawn_id].rarity;
                                uint8_t spawn_r = petal_r;
                                if (spawn_r >= RarityID::kNumRarities) spawn_r = base_r;
                                // Square mob is baked Unique — still scale from the petal tier.
                                if (!is_square && spawn_r < base_r) spawn_r = base_r;
                                mob.set_rarity(spawn_r);
                                if (is_stick) {
                                    float const st = kStickSummonStat[spawn_r];
                                    mob.max_health = st;
                                    mob.health = st;
                                    mob.damage = st;
                                    // Cap body scale — full Unique ×7 radius is what crashed mobiles.
                                    float rs = mob_spawn_radius_mult(base_r, spawn_r);
                                    if (rs > 1.35f) rs = 1.35f;
                                    if (rs != 1.f) {
                                        float nr = mob.get_radius() * rs;
                                        mob.set_radius(nr);
                                        mob.mass = (1 + nr / BASE_FLOWER_RADIUS)
                                            * (MOB_DATA[spawn_id].attributes.stationary ? 10000.f : 1.f);
                                    }
                                } else if (is_pharaoh) {
                                    // Client mummy art keys off this name (EntityFlags not networked).
                                    if (mob.has_component(kName))
                                        mob.set_name("Undead Beetle");
                                    mob.max_health = kPharaohHp[spawn_r];
                                    mob.health = mob.max_health;
                                    mob.damage = kPharaohDmg[spawn_r];
                                    float rs = mob_spawn_radius_mult(base_r, spawn_r);
                                    if (rs > 1.5f) rs = 1.5f;
                                    if (rs != 1.f) {
                                        float nr = mob.get_radius() * rs;
                                        mob.set_radius(nr);
                                        mob.mass = (1 + nr / BASE_FLOWER_RADIUS)
                                            * (MOB_DATA[spawn_id].attributes.stationary ? 10000.f : 1.f);
                                    }
                                } else if (is_square) {
                                    mob.max_health = kSquareHp[spawn_r];
                                    mob.health = mob.max_health;
                                    mob.damage = kSquareDmg[spawn_r];
                                    // Hatch squares move and fight; wild Square stays a rock.
                                    float rs = 1.f + 0.12f * (float) spawn_r;
                                    if (rs > 2.1f) rs = 2.1f;
                                    float nr = mob.get_radius() * rs;
                                    mob.set_radius(nr);
                                    mob.mass = 1.f + nr / BASE_FLOWER_RADIUS;
                                } else {
                                    float th = mob_spawn_threat_mult(base_r, spawn_r);
                                    mob.max_health *= th;
                                    mob.health = mob.max_health;
                                    mob.damage *= mob_spawn_damage_mult(base_r, spawn_r);
                                    if (mob.damage <= 0.f)
                                        mob.damage = 1.f;
                                    float rs = mob_spawn_radius_mult(base_r, spawn_r);
                                    if (rs != 1.f) {
                                        float nr = mob.get_radius() * rs;
                                        mob.set_radius(nr);
                                        mob.mass = (1 + nr / BASE_FLOWER_RADIUS)
                                            * (MOB_DATA[spawn_id].attributes.stationary ? 10000.f : 1.f);
                                    }
                                }
                            });

                            if (petal_data.attributes.spawn_count == 0) {
                                petal_slot.ent_id = mob.id;
                                sim->request_delete(petal.id);
                                break;
                            }
                            game_tick_t life = (is_stick || is_pharaoh)
                                ? (game_tick_t)(6 * TPS)
                                : (game_tick_t)(sec_reload_ticks * petal_data.attributes.spawn_count);
                            entity_set_despawn_tick(mob, life);
                            petal.secondary_reload = 0;
                            if (is_stick) ++stick_summons;
                            // Parent stays the flower (set above); do not rebind to petal.
                        }
                        // Still consume the pulse when capped so sticks don't busy-spin.
                        if (is_stick && summon_n == 0)
                            petal.secondary_reload = 0;
                    }
                } else {
                    //if petal is a mob, or detached (IsDespawning)
                    if (BitMath::at(petal.flags, EntityFlags::kIsDespawning))
                        petal_slot.ent_id = NULL_ENTITY;
                    if (BitMath::at(petal.flags, EntityFlags::kIsDetached))
                        --rot_pos;
                }
            }
            //spread out
            if (petal_data.attributes.clump_radius == 0) ++rot_pos;
        }
        //clump
        if (petal_data.attributes.clump_radius > 0) ++rot_pos;
        player.set_loadout_reloads(i, min_reload * 255);
        // Alive petals only — while reloading (none out) keep full so CD pie owns the cue.
        float slot_hp = health_n > 0 ? health_sum / (float) health_n : 1.f;
        if (slot_hp < 0.f) slot_hp = 0.f;
        if (slot_hp > 1.f) slot_hp = 1.f;
        player.set_loadout_healths(i, (uint8_t) (slot_hp * 255.f + 0.5f));
    };
    if (BitMath::at(player.input, InputFlags::kAttacking)) 
        player.set_face_flags(player.get_face_flags() | (1 << FaceFlags::kAttacking));
    else if (BitMath::at(player.input, InputFlags::kDefending))
        player.set_face_flags(player.get_face_flags() | (1 << FaceFlags::kDefending));
    if (player.poison_ticks > 0)
        player.set_face_flags(player.get_face_flags() | (1 << FaceFlags::kPoisoned));
    if (player.dandy_ticks > 0)
        player.set_face_flags(player.get_face_flags() | (1 << FaceFlags::kDandelioned));
    float rot_speed = (BASE_PETAL_ROTATION_SPEED + buffs.extra_rot + talent_rot) * rot_mult;
    if (buffs.yinyang_count != MAX_SLOT_COUNT) {
        switch (buffs.yinyang_count % 3) {
            case 0:
                player.heading_angle += rot_speed / TPS;
                break;
            case 1:
                player.heading_angle -= rot_speed / TPS;
                break;
            default:
                break;
        }
    } else
        player.heading_angle += 10 * rot_speed / TPS;
}
