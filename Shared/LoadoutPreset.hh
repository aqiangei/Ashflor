#pragma once

#include <Shared/Recipe.hh>
#include <Shared/StaticData.hh>

#include <array>
#include <cstdint>
#include <cstring>

inline constexpr uint8_t LOADOUT_PRESET_COUNT = 20;

struct LoadoutPreset {
    uint8_t slot_count = 0; // 0 means unused
    std::array<PetalID::T, MAX_SLOT_COUNT> petals = {};
};

// Resolve each saved primary slot from owned petals (inventory only):
//   1) exact id  2) strongest lower-rarity same English name  3) leave empty
// Never blocks the switch. Surplus copies refill secondary/bag.
inline bool arrange_loadout_preset(
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> &inventory,
    uint8_t active_slots,
    LoadoutPreset const &preset,
    std::array<uint32_t, PetalID::kNumPetals> &overflow,
    uint8_t &fallback_count
) {
    if (preset.slot_count == 0 || preset.slot_count > MAX_SLOT_COUNT
        || active_slots == 0 || active_slots > MAX_SLOT_COUNT) return false;
    overflow.fill(0);
    fallback_count = 0;
    std::array<uint8_t, PetalID::kNumPetals> available = {};
    uint8_t const end = active_slots + MAX_SLOT_COUNT;
    for (uint8_t i = 0; i < end; ++i)
        if (inventory[i] != PetalID::kNone && inventory[i] < PetalID::kNumPetals)
            ++available[inventory[i]];
    std::array<PetalID::T, 2 * MAX_SLOT_COUNT> arranged = {};
    arranged.fill(PetalID::kNone);
    uint8_t const count = preset.slot_count < active_slots ? preset.slot_count : active_slots;
    for (uint8_t i = 0; i < count; ++i) {
        PetalID::T desired = preset.petals[i];
        if (desired == PetalID::kNone || desired >= PetalID::kNumPetals || Recipe::is_bag_only(desired)) continue;
        PetalID::T chosen = available[desired] ? desired : PetalID::kNone;
        if (chosen == PetalID::kNone) {
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
        if (chosen != desired) ++fallback_count;
        arranged[i] = chosen;
        if (chosen != PetalID::kNone) --available[chosen];
    }
    // Keep newly unlocked primary slots equipped when possible.
    for (uint8_t i = count; i < active_slots; ++i) {
        PetalID::T id = inventory[i];
        if (id != PetalID::kNone && id < PetalID::kNumPetals && !Recipe::is_bag_only(id) && available[id]) {
            arranged[i] = id;
            --available[id];
        }
    }
    // Reuse the old inventory order for leftover copies.
    uint8_t next = count;
    for (uint8_t i = 0; i < end; ++i) {
        PetalID::T id = inventory[i];
        if (id == PetalID::kNone || id >= PetalID::kNumPetals || !available[id]) continue;
        while (next < end && arranged[next] != PetalID::kNone) ++next;
        // The saved primary slots intentionally left blank must stay blank.
        if (next < count) next = count;
        while (next < end && arranged[next] != PetalID::kNone) ++next;
        if (next < end) arranged[next++] = id;
        else {
            // A completely full inventory can have a missing preset slot.
            // Keep the owned petal there instead of creating hidden storage.
            uint8_t hole = 0;
            while (hole < count && arranged[hole] != PetalID::kNone) ++hole;
            if (hole < count) arranged[hole] = id;
            else ++overflow[id];
        }
        --available[id];
    }
    inventory = arranged;
    return true;
}
