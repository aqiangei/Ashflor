#pragma once

#include <Shared/StaticDefinitions.hh>
#include <Helpers/Math.hh>

#include <array>
#include <cstdint>

// Reference ×3-per-tier table (community florr feel). Petal ladders in PETAL_DATA are
// already baked with this curve — do NOT multiply again at spawn. Use for tools /
// new petals authored at Common base, or zone/UI display helpers.
// Common→1 … Mythic→243; Ultra≈500; Super≈1000; Unique≈2000.
inline float rarity_combat_mult(uint8_t rarity) {
    static float const kMult[RarityID::kNumRarities] = {
        1.f, 3.f, 9.f, 27.f, 81.f, 243.f, 500.f, 1000.f, 2000.f
    };
    if (rarity >= RarityID::kNumRarities) return 1.f;
    return kMult[rarity];
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
    // Charcoal / gunmetal — Unique（唯一）top tier; petals, mobs, map rings share this.
    // Mid-steel so nameplates stay readable with the black stroke bake.
    0xff5a616c  // Unique (唯一)
}};

// Zone difficulty 0..MAX_DIFFICULTY == RarityID. Combat follows petal ×3 curve;
// damage stays milder than HP so contact does not instantly delete flowers.
inline float zone_threat_mult(uint32_t difficulty) {
    static float const k[] = {
        1.f, 3.f, 9.f, 27.f, 81.f, 243.f, 500.f, 1000.f, 2000.f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

inline float zone_damage_mult(uint32_t difficulty) {
    static float const k[] = {
        1.0f, 1.15f, 1.35f, 1.55f, 1.80f, 2.10f, 2.40f, 2.75f, 3.20f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Zone XP — mild curve across 9 rarity rings. MUST stay far below combat ×3.
// 2026-10 balance: soft top so outer rings are not ~5.5× soft-zone XP.
inline float zone_xp_mult(uint32_t difficulty) {
    static float const k[] = {
        1.00f, 1.20f, 1.40f, 1.65f, 2.00f, 2.40f, 2.90f, 3.40f, 4.00f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Coin yield by zone — milder than XP; hard zones fund shop without printing money.
// 2026-10 economy Seg4: cap ~2.7 after high-rarity spawn/drop visibility bump.
inline float zone_coin_mult(uint32_t difficulty) {
    static float const k[] = {
        1.00f, 1.10f, 1.25f, 1.45f, 1.70f, 1.95f, 2.20f, 2.45f, 2.70f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Drop rarity tilt defaults per zone tier (ZoneDefinition::drop_multiplier).
// 2026-10 economy Seg2: outer rings slightly more Epic+ tilt (with spawn visibility).
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
        1.00f, // Common
        1.25f, // Unusual
        1.60f, // Rare
        2.10f, // Epic
        2.70f, // Legendary
        3.40f, // Mythic
        1.00f, // Ultra (boss bases are huge)
        1.00f, // Super
        1.00f, // Unique
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

// HP scale from baked MobData rarity → live spawn rarity (petal ×3 curve).
inline float mob_spawn_threat_mult(uint8_t base_rarity, uint8_t spawn_rarity) {
    float base = rarity_combat_mult(base_rarity);
    if (base < 1e-3f) base = 1.f;
    return rarity_combat_mult(spawn_rarity) / base;
}

// Applied at kill grant (Death::_add_mob_xp). 2026-10 economy Seg3: 15→12 after
// Seg1 raised Mythic+ visibility (outer Unique~xp8 kill was ~1344 score).
inline float kill_xp_grant_mult() { return 12.f; }

// Soft XP/loot scale base→spawn. Do NOT reuse mob_spawn_threat_mult (combat ×3):
// zone_xp_mult × combat threat made Ultra commons drop ~2e6 score each.
// Seg3: soft-cap Super/Unique spawn XP slightly.
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

// Milder damage scale so higher zones chip petals without one-shots.
inline float mob_spawn_damage_mult(uint8_t base_rarity, uint8_t spawn_rarity) {
    float th = mob_spawn_threat_mult(base_rarity, spawn_rarity);
    // compress toward 1: sqrt-ish
    return th > 0.f ? (1.f + (th - 1.f) * 0.35f) : 1.f;
}

// Physics + draw radius by spawn rarity (Common 1.00 → Unique 2.00).
// Kept shallow: base radii already include "Massive"/boss bulk, and
// MOB_RADIUS_MULTIPLIER (1.25) stacks on top — Legendary×2 on a r=75 mob
// was filling the screen. Combat HP/damage stay on their own tables.
inline float mob_rarity_radius_mult(uint8_t rarity) {
    static float const k[RarityID::kNumRarities] = {
        1.00f, // Common
        1.12f, // Unusual
        1.25f, // Rare
        1.38f, // Epic
        1.50f, // Legendary
        1.62f, // Mythic
        1.75f, // Ultra
        1.88f, // Super
        2.00f  // Unique
    };
    if (rarity >= RarityID::kNumRarities) return 1.f;
    return k[rarity];
}

// Soft (Common/Unusual): more mix of low tiers so hub packs feel varied.
// Mid: primary ~70%, −1 ~20%, +1 ~10%.
// Legendary+: primary ~60%, −1 ~20%, +1 ~20%.
inline uint8_t pick_zone_spawn_rarity(uint32_t zone_difficulty) {
    uint32_t d = zone_difficulty;
    if (d > MAX_DIFFICULTY) d = MAX_DIFFICULTY;
    float r = frand();
    float primary = 0.70f;
    float down_end = 0.90f; // primary + down
    // Hub / soft belt: mix Common · Unusual · Rare so packs look varied (still low-tier).
    if (d == 0) {
        if (r < 0.42f) return (uint8_t)RarityID::kCommon;
        if (r < 0.78f) return (uint8_t)RarityID::kUnusual;
        return (uint8_t)RarityID::kRare;
    }
    if (d == RarityID::kUnusual) {
        if (r < 0.35f) return (uint8_t)RarityID::kCommon;
        if (r < 0.70f) return (uint8_t)RarityID::kUnusual;
        return (uint8_t)RarityID::kRare;
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
