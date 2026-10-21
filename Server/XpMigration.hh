#pragma once

#include <cstdint>

// One-shot boot migration for XP inflation (zone_xp × combat threat).
// See todo/xp-inflation-migration-plan.md — never wipes petals/coins/crafts.
namespace XpMigration {
    // Returns number of accounts whose level was rewritten. Idempotent via sentinel file.
    uint32_t run_once();
}
