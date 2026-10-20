#pragma once

#include <cstdint>
#include <string>

// Lightweight in-memory guards. Thresholds are generous so normal play,
// reconnects, and shared NAT (school/home) are unaffected.
namespace RateLimit {
    // Call from the game tick occasionally (~every few seconds).
    void tick_prune();

    // Connection caps (per client IP). Empty IP → skip IP limits.
    uint8_t allow_connect(std::string const &ip);
    void on_connect(std::string const &ip);
    void on_disconnect(std::string const &ip);

    // Auth precheck / bookkeeping. mode = AccountAuthMode.
    // Returns AccountResultCode::kAccountOk or kAccountRateLimited.
    uint8_t precheck_auth(std::string const &ip, std::string const &username, uint8_t mode);
    void note_auth_fail(std::string const &ip, std::string const &username, uint8_t mode);
    void note_auth_ok(std::string const &ip, std::string const &username);
}
