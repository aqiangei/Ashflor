#pragma once

#include <Shared/StaticDefinitions.hh>
#include <Helpers/Math.hh>

#include <array>
#include <cstdint>

// Reference ×3-per-tier table (community florr feel). Petal ladders in PETAL_DATA are
// already baked with this curve — do NOT multiply again at spawn. Use for tools /
// new petals authored at Common base, or zone/UI display helpers.
// Common→1 … Mythic→243; Unique≈500; Super≈1000; Ultra≈2000.
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
    0xffde1f65, // Unique
    0xffff2b75, // Super
    0xfff70fb6  // Ultra
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

inline float zone_xp_mult(uint32_t difficulty) {
    static float const k[] = {
        1.f, 2.5f, 6.f, 14.f, 32.f, 70.f, 130.f, 220.f, 360.f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Coin yield by zone — milder than XP so hard zones fund shop without printing money.
inline float zone_coin_mult(uint32_t difficulty) {
    static float const k[] = {
        1.0f, 1.5f, 2.25f, 3.5f, 5.5f, 8.5f, 12.f, 17.f, 24.f
    };
    if (difficulty > MAX_DIFFICULTY) difficulty = MAX_DIFFICULTY;
    return k[difficulty];
}

// Drop rarity tilt defaults per zone tier (ZoneDefinition::drop_multiplier).
inline float zone_drop_tilt(uint32_t difficulty) {
    static float const k[] = {
        0.02f, 0.06f, 0.10f, 0.18f, 0.28f, 0.40f, 0.52f, 0.62f, 0.72f
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
        1.00f, // Unique (boss bases are huge)
        1.00f, // Super
        1.00f, // Ultra
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

// Milder damage scale so higher zones chip petals without one-shots.
inline float mob_spawn_damage_mult(uint8_t base_rarity, uint8_t spawn_rarity) {
    float th = mob_spawn_threat_mult(base_rarity, spawn_rarity);
    // compress toward 1: sqrt-ish
    return th > 0.f ? (1.f + (th - 1.f) * 0.35f) : 1.f;
}

// Zone primary (~70%), adjacent lower (~20%), adjacent higher (~10%).
inline uint8_t pick_zone_spawn_rarity(uint32_t zone_difficulty) {
    uint32_t d = zone_difficulty;
    if (d > MAX_DIFFICULTY) d = MAX_DIFFICULTY;
    float r = frand();
    if (r < 0.70f) return (uint8_t)d;
    if (r < 0.90f) return (uint8_t)(d > 0 ? d - 1 : 0);
    if (d < MAX_DIFFICULTY) return (uint8_t)(d + 1);
    return (uint8_t)d;
}
