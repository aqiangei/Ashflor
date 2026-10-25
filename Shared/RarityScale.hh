#pragma once

#include <Shared/StaticDefinitions.hh>
#include <Helpers/Math.hh>

#include <array>
#include <cmath>
#include <cstdint>

// Wiki florr rarity ladders (Eternal folded into Unique). One source of truth:
// petals bake PETAL_COMBAT / PETAL_HEAL into PETAL_DATA; live mobs scale with
// MOB_HP / MOB_DMG relative to baked MobData.rarity.

// Petal combat + mob contact damage: ×3 per tier → Unique = 6561× Common.
inline float rarity_combat_mult(uint8_t rarity) {
    static float const kMult[RarityID::kNumRarities] = {
        1.f, 3.f, 9.f, 27.f, 81.f, 243.f, 729.f, 2187.f, 6561.f
    };
    if (rarity >= RarityID::kNumRarities) return 1.f;
    return kMult[rarity];
}

// Heal / armor-style petal attrs: ×3 to Mythic, then ×√3 per tier.
inline float rarity_heal_mult(uint8_t rarity) {
    static float const kSqrt3 = 1.73205080757f;
    static float const kMult[RarityID::kNumRarities] = {
        1.f,
        3.f,
        9.f,
        27.f,
        81.f,
        243.f,
        243.f * kSqrt3,             // Ultra ≈ 420.889
        729.f,                      // Super
        729.f * kSqrt3              // Unique ≈ 1262.665
    };
    if (rarity >= RarityID::kNumRarities) return 1.f;
    return kMult[rarity];
}

// Mob HP vs petal combat (×3/tier → Unique 6561×). Through Mythic stays
// wiki-steep; Ultra+ used to jump to 145800/4e6/2.6e7 so Unique petals could
// not meaningfully chip Ultra trash. Retuned so Unique≈4× Ultra HP pad,
// Super≈21×, Unique-mob≈100× (bosses add a separate pad in Simulation).
inline float rarity_mob_hp_mult(uint8_t rarity) {
    static float const kMult[RarityID::kNumRarities] = {
        1.f,
        3.75f,
        13.5f,
        54.f,
        324.f,
        3159.f,
        28000.f,   // Ultra (was 145800)
        140000.f,  // Super (was 4374000)
        700000.f   // Unique (was 26244000)
    };
    if (rarity >= RarityID::kNumRarities) return 1.f;
    return kMult[rarity];
}

// Mob damage tracks petal combat so same-rarity contact is matched.
inline float rarity_mob_damage_mult(uint8_t rarity) {
    return rarity_combat_mult(rarity);
}

// Petal / zone rarity paint (ARGB). Shared so MapGen rings match G-key minimap.
inline std::array<uint32_t, RarityID::kNumRarities> const RARITY_COLORS = {{
    0xff7eef6d, // Common
    0xffffe65d, // Unusual
    0xff4d52e3, // Rare
    0xff861fde, // Epic
    0xffde1f1f, // Legendary
    0xff1fdbde, // Mythic
    0xfff70fb6, // Ultra (究极)
    // Mint / aqua-green — green-family like florr Super (#2bffa4), not Common lime.
    0xff2bffa4, // Super (超凡)
    // Bright platinum — Unique（唯一）top tier.
    0xffc8d0dc  // Unique (唯一)
}};

// Zone difficulty 0..MAX_DIFFICULTY == RarityID. Combat display uses petal ×3.
inline float zone_threat_mult(uint32_t difficulty) {
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return rarity_combat_mult((uint8_t)difficulty);
}

inline float zone_damage_mult(uint32_t difficulty) {
    static float const k[] = {
        1.0f, 1.15f, 1.35f, 1.55f, 1.80f, 2.10f, 2.40f, 2.75f, 3.20f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Zone XP — mild curve; MUST stay far below combat ×3.
inline float zone_xp_mult(uint32_t difficulty) {
    static float const k[] = {
        1.00f, 1.20f, 1.40f, 1.65f, 2.00f, 2.40f, 2.90f, 3.40f, 4.00f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Coin yield by zone — milder than XP.
inline float zone_coin_mult(uint32_t difficulty) {
    static float const k[] = {
        1.00f, 1.10f, 1.25f, 1.45f, 1.70f, 1.95f, 2.20f, 2.45f, 2.70f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Drop rarity tilt defaults per zone tier.
inline float zone_drop_tilt(uint32_t difficulty) {
    static float const k[] = {
        0.02f, 0.05f, 0.09f, 0.15f, 0.22f, 0.32f, 0.42f, 0.55f, 0.65f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Soft species-rarity HP bump when comparing baked MOB_DATA.rarity. Prefer
// spawn-rarity relative scaling in Map::_spawn_mob_at instead.
inline float mob_rarity_threat_mult(uint8_t rarity) {
    static float const k[RarityID::kNumRarities] = {
        1.00f, 1.25f, 1.60f, 2.10f, 2.70f, 3.40f, 1.00f, 1.00f, 1.00f
    };
    if (rarity >= RarityID::kNumRarities) return 1.f;
    return k[rarity];
}

inline float mob_rarity_damage_mult(uint8_t rarity) {
    static float const k[RarityID::kNumRarities] = {
        1.00f, 1.10f, 1.20f, 1.35f, 1.50f, 1.65f, 1.00f, 1.00f, 1.00f
    };
    if (rarity >= RarityID::kNumRarities) return 1.f;
    return k[rarity];
}

// HP scale from baked MobData rarity → live spawn rarity (wiki mob HP table).
inline float mob_spawn_threat_mult(uint8_t base_rarity, uint8_t spawn_rarity) {
    float base = rarity_mob_hp_mult(base_rarity);
    if (base < 1e-3f) base = 1.f;
    return rarity_mob_hp_mult(spawn_rarity) / base;
}

// Applied at kill grant (Death::_add_mob_xp). Keep mild — combat HP is huge.
inline float kill_xp_grant_mult() { return 12.f; }

// Soft XP/loot scale base→spawn. Do NOT reuse mob_spawn_threat_mult.
inline float mob_spawn_xp_mult(uint8_t base_rarity, uint8_t spawn_rarity) {
    static float const k[RarityID::kNumRarities] = {
        1.00f, 1.12f, 1.28f, 1.48f, 1.72f, 2.00f, 2.30f, 2.40f, 2.55f
    };
    uint8_t b = base_rarity < RarityID::kNumRarities ? base_rarity : 0;
    uint8_t s = spawn_rarity < RarityID::kNumRarities ? spawn_rarity : 0;
    float base = k[b];
    if (base < 1e-3f) base = 1.f;
    return k[s] / base;
}

// Damage scale tracks petal combat (same-rarity flower vs mob matched).
inline float mob_spawn_damage_mult(uint8_t base_rarity, uint8_t spawn_rarity) {
    float base = rarity_mob_damage_mult(base_rarity);
    if (base < 1e-3f) base = 1.f;
    return rarity_mob_damage_mult(spawn_rarity) / base;
}

// Physics + draw radius ladder (Common 1× → Unique ~7×). Aligns SpatialHash /
// changelog; combat HP/damage stay on their own tables.
inline float mob_rarity_radius_mult(uint8_t rarity) {
    static float const k[RarityID::kNumRarities] = {
        1.00f, // Common
        1.20f, // Unusual
        1.50f, // Rare
        1.90f, // Epic
        2.50f, // Legendary
        3.30f, // Mythic
        4.40f, // Ultra
        5.60f, // Super
        7.00f  // Unique
    };
    if (rarity >= RarityID::kNumRarities) return 1.f;
    return k[rarity];
}

// Live spawn vs baked MobData.rarity (Unique-anchored mobs stay 1× when
// spawn_r == base_r; Common templates grow to Unique ~7×).
inline float mob_spawn_radius_mult(uint8_t base_rarity, uint8_t spawn_rarity) {
    float base = mob_rarity_radius_mult(base_rarity);
    if (base < 1e-3f) base = 1.f;
    return mob_rarity_radius_mult(spawn_rarity) / base;
}

// Soft (Common/Unusual): more mix of low tiers so hub packs feel varied.
inline uint8_t zone_spawn_rarity_cap(uint32_t difficulty) {
    if (difficulty >= RarityID::kUnique) return RarityID::kUnique;
    // Starter belts contain only their own tier and below.
    if (difficulty <= RarityID::kUnusual) return (uint8_t)difficulty;
    return (uint8_t)std::min<uint32_t>(RarityID::kSuper, difficulty + 1);
}

// Mid: primary ~70%, −1 ~20%, +1 ~10%.
// Legendary+: primary ~60%, −1 ~20%, +1 ~20%.
inline uint8_t pick_zone_spawn_rarity(uint32_t zone_difficulty) {
    uint32_t d = zone_difficulty;
    if (d > MAX_DIFFICULTY) d = MAX_DIFFICULTY;
    float r = frand();
    float primary = 0.70f;
    float down_end = 0.90f; // primary + down
    // Common hub stays Common. Unusual belt mixes only Common and Unusual.
    if (d == 0) {
        return (uint8_t)RarityID::kCommon;
    }
    if (d == RarityID::kUnusual) {
        if (r < 0.35f) return (uint8_t)RarityID::kCommon;
        return (uint8_t)RarityID::kUnusual;
    }
    if (d >= RarityID::kLegendary) {
        primary = 0.60f;
        down_end = 0.80f;
    }
    if (r < primary) return (uint8_t)d;
    if (r < down_end) return (uint8_t)(d > 0 ? d - 1 : 0);
    if (d < MAX_DIFFICULTY) return (uint8_t)(d + 1);
    return (uint8_t)d;
}
