#pragma once

#include <Shared/StaticDefinitions.hh>

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

// Zone difficulty 0..MAX_DIFFICULTY → HP threat multiplier
inline float zone_threat_mult(uint32_t difficulty) {
    return 1.f + 0.55f * (float) difficulty;
}

inline float zone_damage_mult(uint32_t difficulty) {
    return 1.f + 0.35f * (float) difficulty;
}

inline float zone_xp_mult(uint32_t difficulty) {
    return 1.f + 0.25f * (float) difficulty;
}
