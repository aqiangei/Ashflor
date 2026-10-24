#pragma once

#include <cstdint>

// One-shot boot migration for XP inflation (zone_xp × combat threat).
// See todo/xp-inflation-migration-plan.md — never wipes petals/coins/crafts.
namespace XpMigration {
    // Returns number of accounts whose level was rewritten. Idempotent via sentinel file.
    uint32_t run_once();
    // Every boot: compress UINT32_MAX / absurd overflow rows (score or level).
    // Idempotent on clean data; needed because v2 sentinel skips re-runs and new
    // guests can still pin UINT32_MAX via the old overflow clamp.
    uint32_t scrub_overflow();
}
