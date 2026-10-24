#pragma once

#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>
#include <Shared/Talent.hh>

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>

inline constexpr uint8_t LOADOUT_PRESET_COUNT = 20;
// Legacy wire prefix; the secondary row is appended as an optional extension.
inline constexpr uint32_t LOADOUT_PRESET_WIDTH =
    1u + MAX_SLOT_COUNT + (uint32_t) TalentID::kNum;
// Pre-talent presets (slot_count + petals only)
inline constexpr uint32_t LOADOUT_PRESET_WIDTH_LEGACY = 1u + MAX_SLOT_COUNT;

struct LoadoutPreset {
    uint8_t slot_count = 0; // 0 means unused
    std::array<PetalID::T, MAX_SLOT_COUNT> petals = {};
    std::array<PetalID::T, MAX_SLOT_COUNT> secondary = {};
    // Legacy wire/disk field — unused. K/L presets share account talent_ranks.
    std::array<uint8_t, TalentID::kNum> talent_ranks = {};
};

inline LoadoutPreset current_loadout_preset(
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> const &inventory,
    uint8_t active_slots
) {
    LoadoutPreset preset;
    if (active_slots == 0 || active_slots > MAX_SLOT_COUNT) return preset;
    preset.slot_count = active_slots;
    for (uint8_t i = 0; i < active_slots; ++i) {
        preset.petals[i] = inventory[i];
        preset.secondary[i] = inventory[active_slots + i];
    }
    return preset;
}

// Autosave real edits to the outgoing preset, including an intentionally empty
// bar. An unchanged, partially resolved apply must not erase its saved targets.
inline void refresh_active_loadout_preset(
    std::array<LoadoutPreset, LOADOUT_PRESET_COUNT> &presets,
    uint8_t &active,
    LoadoutPreset &last_applied,
    LoadoutPreset const &current
) {
    if (current.slot_count == 0) return;
    if (active >= LOADOUT_PRESET_COUNT) {
        if (presets[0].slot_count != 0) return;
        active = 0;
        presets[active] = current;
    } else if (current.slot_count != last_applied.slot_count
        || current.petals != last_applied.petals
        || current.secondary != last_applied.secondary) {
        presets[active] = current;
    }
    last_applied = current;
}

// Resolve from the whole collection, as in florr_clone-web's equipPreset:
// reserve every exact match in both rows before trying lower rarities.
// Unequipped gear returns to backpack stacks; legacy hidden bag slots stay put.
// Work on copies so an invalid preset or unrepresentable stack changes nothing.
inline bool arrange_loadout_preset(
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> &inventory,
    uint8_t active_slots,
    LoadoutPreset const &preset,
    std::array<uint32_t, PetalID::kNumPetals> &bank,
    uint8_t &fallback_count
) {
    if (preset.slot_count > MAX_SLOT_COUNT
        || active_slots == 0 || active_slots > MAX_SLOT_COUNT) return false;
    uint8_t adjusted = 0;
    std::array<uint64_t, PetalID::kNumPetals> available = {};
    for (uint32_t id = 1; id < PetalID::kNumPetals; ++id) available[id] = bank[id];
    uint8_t const end = active_slots + MAX_SLOT_COUNT;
    for (uint8_t i = 0; i < end; ++i)
        if (inventory[i] != PetalID::kNone && inventory[i] < PetalID::kNumPetals)
            ++available[inventory[i]];
    auto arranged = inventory;
    for (uint8_t i = 0; i < end; ++i) arranged[i] = PetalID::kNone;
    uint8_t const count = preset.slot_count != 0 && preset.slot_count < active_slots
        ? preset.slot_count : active_slots;
    auto desired_at = [&](uint8_t i) {
        return i < count ? preset.petals[i] : preset.secondary[i - count];
    };
    auto slot_at = [&](uint8_t i) {
        return i < count ? i : (uint8_t) (active_slots + i - count);
    };
    // Reserve exact matches across both rows before any rarity fallback.
    for (uint8_t i = 0; i < 2 * count; ++i) {
        PetalID::T desired = desired_at(i);
        if (desired == PetalID::kNone || desired >= PetalID::kNumPetals
            || Recipe::is_bag_only(desired) || !available[desired]) continue;
        arranged[slot_at(i)] = desired;
        --available[desired];
    }
    for (uint8_t i = 0; i < 2 * count; ++i) {
        PetalID::T desired = desired_at(i);
        if (desired == PetalID::kNone || desired >= PetalID::kNumPetals || Recipe::is_bag_only(desired)) continue;
        if (arranged[slot_at(i)] != PetalID::kNone) continue;
        PetalID::T chosen = PetalID::kNone;
        {
            // Walk rarity downward: Ultra→…→Common, same name only.
            uint8_t rarity = PETAL_DATA[desired].rarity;
            char const *family = PETAL_DATA[desired].name;
            while (rarity > 0 && chosen == PetalID::kNone) {
                --rarity;
                for (uint32_t id = 1; id < PetalID::kNumPetals; ++id) {
                    if (!available[id] || PETAL_DATA[id].rarity != rarity) continue;
                    if (family == nullptr || PETAL_DATA[id].name == nullptr) continue;
                    if (std::strcmp(PETAL_DATA[id].name, family) != 0) continue;
                    chosen = (PetalID::T) id;
                    break;
                }
            }
        }
        // Missing entirely → leave the slot empty and keep switching.
        ++adjusted;
        arranged[slot_at(i)] = chosen;
        if (chosen != PetalID::kNone) --available[chosen];
    }
    auto preserve = [&](uint8_t i) {
        PetalID::T id = inventory[i];
        if (id != PetalID::kNone && id < PetalID::kNumPetals && available[id]) {
            arranged[i] = id;
            --available[id];
        }
    };
    // An older preset has no targets for newly unlocked pairs.
    for (uint8_t i = count; i < active_slots; ++i) {
        if (!Recipe::is_bag_only(inventory[i])) preserve(i);
        if (!Recipe::is_bag_only(inventory[active_slots + i])) preserve(active_slots + i);
    }
    for (uint8_t i = 2 * active_slots; i < end; ++i) preserve(i);
    auto next_bank = bank;
    for (uint32_t id = 1; id < PetalID::kNumPetals; ++id) {
        if (available[id] > std::numeric_limits<uint32_t>::max()) return false;
        // This transfers already-owned copies; applying the acquisition cap
        // here would destroy equipped petals when a stack is full.
        next_bank[id] = (uint32_t) available[id];
    }
    inventory = arranged;
    bank = next_bank;
    fallback_count = adjusted;
    return true;
}
