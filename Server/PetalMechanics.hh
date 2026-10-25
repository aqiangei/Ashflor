#pragma once

#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

// Resolve from the requested loadout, so damage, healing and orbit behavior
// agree even on the tick of an equipment edit. Missing rungs fall down only.
inline PetalID::T effective_petal_id(Entity const &flower, uint32_t slot) {
    if (slot >= flower.get_loadout_count()) return PetalID::kNone;
    PetalID::T id = flower.get_loadout_ids(slot);
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return PetalID::kNone;
    if (!PETAL_DATA[id].attributes.mimic_left) return id;
    uint32_t n = flower.get_loadout_count();
    if (n < 2) return PetalID::kNone;
    auto left = flower.get_loadout_ids(slot == 0 ? n - 1 : slot - 1);
    if (left == PetalID::kNone || left >= PetalID::kNumPetals) return PetalID::kNone;
    if (PETAL_DATA[left].attributes.mimic_left || PETAL_DATA[left].attributes.corruption)
        return PetalID::kNone;
    for (int rarity = PETAL_DATA[id].rarity; rarity >= 0; --rarity) {
        auto target = Craft::petal_at_rarity(left, (uint8_t)rarity);
        if (target != PetalID::kNone && target < PetalID::kNumPetals) return target;
    }
    return PetalID::kNone;
}

inline bool petal_projectile(Entity const &ent) {
    return ent.has_component(kPetal)
        && (BitMath::at(ent.flags, EntityFlags::kIsDespawning)
            || ent.get_split_projectile());
}
