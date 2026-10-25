// Regression tests for K/L switching. Run tools/test_loadout_presets.sh.
#include <Shared/LoadoutPreset.hh>

#include <cassert>
#include <cstdio>
#include <random>

using Inventory = std::array<PetalID::T, 2 * MAX_SLOT_COUNT>;
using Bank = std::array<uint32_t, PetalID::kNumPetals>;

static auto owned(Inventory const &inventory, Bank const &bank) {
    std::array<uint64_t, PetalID::kNumPetals> counts = {};
    for (uint32_t id = 1; id < PetalID::kNumPetals; ++id) counts[id] = bank[id];
    for (auto id : inventory)
        if (id != PetalID::kNone && id < PetalID::kNumPetals) ++counts[id];
    return counts;
}

static uint8_t apply(Inventory &inventory, Bank &bank, LoadoutPreset const &preset,
                     uint8_t slots = MAX_SLOT_COUNT) {
    auto before = owned(inventory, bank);
    uint8_t adjusted = 255;
    assert(arrange_loadout_preset(inventory, slots, preset, bank, adjusted));
    assert(owned(inventory, bank) == before);
    return adjusted;
}

static void round_trip_full_bars() {
    Inventory inventory;
    inventory.fill(PetalID::kRose);
    for (uint8_t i = 0; i < MAX_SLOT_COUNT; ++i) inventory[i] = PetalID::kBasic;
    auto const original = inventory;
    auto k = current_loadout_preset(inventory, MAX_SLOT_COUNT);
    LoadoutPreset l = k;
    l.petals.fill(PetalID::kLight);
    l.secondary.fill(PetalID::kHeavy);
    Bank bank = {};
    bank[PetalID::kLight] = MAX_SLOT_COUNT;
    bank[PetalID::kHeavy] = MAX_SLOT_COUNT;
    auto const initial_bank = bank;
    for (int i = 0; i < 1000; ++i) {
        assert(apply(inventory, bank, l) == 0);
        for (uint8_t j = 0; j < MAX_SLOT_COUNT; ++j) {
            assert(inventory[j] == PetalID::kLight);
            assert(inventory[MAX_SLOT_COUNT + j] == PetalID::kHeavy);
        }
        assert(bank[PetalID::kBasic] == MAX_SLOT_COUNT);
        assert(bank[PetalID::kRose] == MAX_SLOT_COUNT);
        assert(apply(inventory, bank, k) == 0);
        assert(inventory == original && bank == initial_bank);
    }
}

static void empty_then_restore_and_stack_caps() {
    for (uint8_t slots = 5; slots <= MAX_SLOT_COUNT; ++slots) {
        Inventory inventory = {};
        for (uint8_t i = 0; i < slots; ++i) inventory[i] = PetalID::kRose;
        // Both full and empty spare rows: outgoing gear always goes to stacks.
        for (bool full : {false, true}) {
            for (uint8_t i = slots; i < slots + MAX_SLOT_COUNT; ++i)
                inventory[i] = full ? PetalID::kLight : PetalID::kNone;
            Bank bank = {};
            bank[PetalID::kRose] = Recipe::bank_stack_cap(PetalID::kRose);
            auto const initial_bank = bank;
            auto const original = inventory;
            auto const preset = current_loadout_preset(inventory, slots);
            apply(inventory, bank, {}, slots);
            for (uint8_t i = 0; i < 2 * slots; ++i) assert(inventory[i] == PetalID::kNone);
            for (uint8_t i = 2 * slots; i < slots + MAX_SLOT_COUNT; ++i)
                assert(inventory[i] == original[i]);
            assert(bank[PetalID::kRose] == initial_bank[PetalID::kRose] + slots);
            assert(bank[PetalID::kLight] == (full ? slots : 0));
            assert(apply(inventory, bank, preset, slots) == 0);
            assert(inventory == original && bank == initial_bank);
        }
    }
}

static void exact_before_fallback_and_missing() {
    // Discover a real three-rarity family from production content.
    PetalID::T low = PetalID::kNone, mid = low, high = low;
    for (uint32_t a = 1; a < PetalID::kNumPetals && high == PetalID::kNone; ++a) {
        for (uint32_t b = 1; b < PetalID::kNumPetals && high == PetalID::kNone; ++b) {
            if (!PETAL_DATA[a].name || !PETAL_DATA[b].name
                || PETAL_DATA[a].rarity >= PETAL_DATA[b].rarity
                || std::strcmp(PETAL_DATA[a].name, PETAL_DATA[b].name)) continue;
            for (uint32_t c = 1; c < PetalID::kNumPetals; ++c) {
                if (PETAL_DATA[c].name && PETAL_DATA[c].rarity > PETAL_DATA[b].rarity
                    && !std::strcmp(PETAL_DATA[a].name, PETAL_DATA[c].name)) {
                    low = (PetalID::T) a; mid = (PetalID::T) b; high = (PetalID::T) c;
                    break;
                }
            }
        }
    }
    assert(high != PetalID::kNone);
    Inventory inventory = {};
    Bank bank = {};
    bank[low] = bank[mid] = 1;
    LoadoutPreset preset;
    preset.slot_count = MAX_SLOT_COUNT;
    preset.petals[0] = high;
    preset.secondary[1] = mid;
    preset.petals[2] = high;
    assert(apply(inventory, bank, preset) == 2);
    assert(inventory[0] == low && inventory[MAX_SLOT_COUNT + 1] == mid
        && inventory[2] == PetalID::kNone);
    // Reapplying a partial preset must stay stable and not consume other gear.
    auto const partial = inventory;
    assert(apply(inventory, bank, preset) == 2);
    assert(inventory == partial);
}

static void spare_sources_and_new_slots() {
    Inventory inventory = {};
    Bank bank = {};
    inventory[8] = PetalID::kRose;
    inventory[14] = PetalID::kLight;
    inventory[18] = PetalID::kBasic;
    LoadoutPreset preset;
    preset.slot_count = 5;
    preset.petals[0] = PetalID::kLight;
    bank[PetalID::kLight] = 1;
    assert(apply(inventory, bank, preset) == 0);
    assert(inventory[0] == PetalID::kLight && inventory[14] == PetalID::kLight);
    assert(inventory[8] == PetalID::kRose);
    assert(inventory[18] == PetalID::kBasic);

    // At each slot count, both rows map to the corresponding saved positions.
    for (uint8_t slots = 5; slots <= MAX_SLOT_COUNT; ++slots) {
        inventory.fill(PetalID::kNone);
        bank.fill(0);
        for (uint8_t i = 0; i < slots; ++i) {
            inventory[i] = i % 2 ? PetalID::kRose : PetalID::kNone;
            inventory[slots + i] = i % 2 ? PetalID::kNone : PetalID::kLight;
        }
        auto const original = inventory;
        auto saved = current_loadout_preset(inventory, slots);
        for (uint8_t i = 0; i < slots; ++i)
            std::swap(inventory[i], inventory[slots + i]);
        assert(apply(inventory, bank, saved, slots) == 0);
        assert(inventory == original);
    }
}

static void older_schemes_cannot_steal_new_secondary_slots() {
    for (uint8_t slots = 6; slots <= MAX_SLOT_COUNT; ++slots) {
        for (bool saved_secondary : {false, true}) {
            Inventory inventory = {};
            Bank bank = {};
            inventory[slots + slots - 1] = PetalID::kCutter;
            inventory[slots + slots - 2] = PetalID::kMagnet;
            auto before = inventory;
            LoadoutPreset preset;
            preset.slot_count = 5;
            preset.petals[0] = PetalID::kCutter;
            preset.petals[1] = PetalID::kMagnet;
            if (saved_secondary) {
                preset.secondary[0] = PetalID::kRose;
                bank[PetalID::kRose] = 1;
            }
            // No spare copy: the old preset must report adjustment, never move
            // an unaddressed secondary copy up into primary.
            assert(apply(inventory, bank, preset, slots) >= 1);
            assert(inventory[slots + slots - 1] == before[slots + slots - 1]);
            if (slots > 6) assert(inventory[slots + slots - 2] == before[slots + slots - 2]);
            // Once another copy exists, both rows can retain the intended item.
            bank[PetalID::kCutter] += 1;
            bank[PetalID::kMagnet] += 1;
            apply(inventory, bank, preset, slots);
            assert(inventory[0] == PetalID::kCutter);
            assert(inventory[slots + slots - 1] == PetalID::kCutter);
        }
    }
}

static void autosave_keeps_target_and_outgoing_bank() {
    std::array<LoadoutPreset, LOADOUT_PRESET_COUNT> presets = {};
    Inventory inventory = {};
    inventory[0] = PetalID::kRose;
    auto current = current_loadout_preset(inventory, MAX_SLOT_COUNT);
    uint8_t active = LOADOUT_PRESET_COUNT;
    LoadoutPreset snapshot;
    refresh_active_loadout_preset(presets, active, snapshot, current);
    assert(active == 0 && presets[0].petals[0] == PetalID::kRose);

    presets[10].slot_count = MAX_SLOT_COUNT;
    presets[10].petals[0] = PetalID::kLight;
    presets[10].secondary[4] = PetalID::kHeavy;
    auto const l1 = presets[10];
    // A manual edit belongs to K1 and must never overwrite the L1 destination.
    inventory[0] = PetalID::kBasic;
    refresh_active_loadout_preset(presets, active, snapshot,
        current_loadout_preset(inventory, MAX_SLOT_COUNT));
    assert(presets[0].petals[0] == PetalID::kBasic && presets[10].petals == l1.petals);
    // Editing only a secondary slot must autosave to the outgoing scheme too.
    inventory[MAX_SLOT_COUNT + 3] = PetalID::kRose;
    refresh_active_loadout_preset(presets, active, snapshot,
        current_loadout_preset(inventory, MAX_SLOT_COUNT));
    assert(presets[0].secondary[3] == PetalID::kRose);
    assert(presets[10].secondary == l1.secondary);

    Bank bank = {};
    apply(inventory, bank, l1); // Light not owned: partial/empty apply.
    active = 10;
    snapshot = current_loadout_preset(inventory, MAX_SLOT_COUNT);
    for (int i = 0; i < 30; ++i)
        refresh_active_loadout_preset(presets, active, snapshot,
            current_loadout_preset(inventory, MAX_SLOT_COUNT));
    assert(presets[10].petals == l1.petals);
    assert(presets[10].secondary == l1.secondary);
    // Login initializes the snapshot from the actual bar, so later capture
    // after reconnect must also preserve the saved targets.
    snapshot = current_loadout_preset(inventory, MAX_SLOT_COUNT);
    refresh_active_loadout_preset(presets, active, snapshot, snapshot);
    assert(presets[10].petals == l1.petals);
    bank[PetalID::kLight] = 1;
    bank[PetalID::kHeavy] = 1;
    assert(apply(inventory, bank, presets[10]) == 0);
    assert(inventory[0] == PetalID::kLight);
    assert(inventory[MAX_SLOT_COUNT + 4] == PetalID::kHeavy);
}

static void blank_secondary_preserves_live_row() {
    // Preset with primary only (legacy / never-saved secondary): switching
    // must not bank the live secondary gear.
    Inventory inventory = {};
    Bank bank = {};
    uint8_t const slots = 8;
    for (uint8_t i = 0; i < slots; ++i) {
        inventory[i] = PetalID::kBasic;
        inventory[slots + i] = (i % 2) ? PetalID::kCutter : PetalID::kHeavy;
    }
    auto const secondary_before = inventory;
    LoadoutPreset primary_only;
    primary_only.slot_count = slots;
    for (uint8_t i = 0; i < slots; ++i)
        primary_only.petals[i] = PetalID::kRose;
    bank[PetalID::kRose] = slots;
    assert(apply(inventory, bank, primary_only, slots) == 0);
    for (uint8_t i = 0; i < slots; ++i) {
        assert(inventory[i] == PetalID::kRose);
        assert(inventory[slots + i] == secondary_before[slots + i]);
    }
    // Explicit empty scheme (slot_count 0) still clears both rows.
    apply(inventory, bank, {}, slots);
    for (uint8_t i = 0; i < 2 * slots; ++i)
        assert(inventory[i] == PetalID::kNone);
}

static void invalid_input_is_atomic() {
    Inventory inventory = {};
    Bank bank = {};
    inventory[0] = PetalID::kRose;
    auto const before = inventory;
    uint8_t adjusted = 99;
    LoadoutPreset invalid;
    invalid.slot_count = MAX_SLOT_COUNT + 1;
    assert(!arrange_loadout_preset(inventory, 5, invalid, bank, adjusted));
    assert(inventory == before && adjusted == 99);
    assert(!arrange_loadout_preset(inventory, 0, {}, bank, adjusted));
    bank[PetalID::kRose] = UINT32_MAX;
    auto const bank_before = bank;
    assert(!arrange_loadout_preset(inventory, 5, {}, bank, adjusted));
    assert(inventory == before && bank == bank_before && adjusted == 99);
}

static void randomized_conservation() {
    std::mt19937 rng(20261024);
    for (int run = 0; run < 100; ++run) {
        Inventory inventory = {};
        Bank bank = {};
        uint8_t const slots = 5 + rng() % 6;
        for (uint8_t i = 0; i < slots + MAX_SLOT_COUNT; ++i)
            inventory[i] = (PetalID::T) (rng() % PetalID::kNumPetals);
        for (uint32_t id = 1; id < PetalID::kNumPetals; ++id) bank[id] = rng() % 5;
        auto const initial = owned(inventory, bank);
        for (int step = 0; step < 100; ++step) {
            LoadoutPreset preset;
            preset.slot_count = step % 11 ? slots : 0;
            for (uint8_t i = 0; i < preset.slot_count; ++i) {
                preset.petals[i] = (PetalID::T) (rng() % PetalID::kNumPetals);
                preset.secondary[i] = (PetalID::T) (rng() % PetalID::kNumPetals);
            }
            apply(inventory, bank, preset, slots);
            assert(owned(inventory, bank) == initial);
        }
    }
}

int main() {
    round_trip_full_bars();
    empty_then_restore_and_stack_caps();
    exact_before_fallback_and_missing();
    spare_sources_and_new_slots();
    autosave_keeps_target_and_outgoing_bank();
    blank_secondary_preserves_live_row();
    older_schemes_cannot_steal_new_secondary_slots();
    invalid_input_is_atomic();
    randomized_conservation();
    std::puts("PASS: both-row K/L round trips, paired/empty slots, stack caps, exact/fallback, "
              "blank-secondary preserve, autosave/reconnect, atomic failure, 10000 random switches");
}
