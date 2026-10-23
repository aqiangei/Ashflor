#pragma once

#include <cstdint>
#include <string>

// Append-only log of Live console petal grants (admin_gives.jsonl).
// Clawback on next boot is not wired yet — record only.
namespace AdminGiveLog {
    void record(std::string const &account, std::string const &who,
                uint32_t petal_id, uint8_t rarity, uint32_t count);
}
