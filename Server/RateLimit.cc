#include <Server/RateLimit.hh>

#include <Shared/Binary.hh>

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace {

// --- Tunables (gameplay-first) ---
// Shared NAT / multi-tab / reconnect races need headroom.
constexpr uint32_t kMaxConnPerIp = 24;

// Wrong-password lock: only after many failures, short cool-down.
constexpr uint32_t kFailLockCount = 15;
constexpr uint64_t kFailLockMs = 45'000;

// IP-wide auth attempt window (login+register+resume floods).
constexpr uint64_t kAuthIpWindowMs = 60'000;
constexpr uint32_t kAuthIpMax = 40;

// Register-only: a few alts OK; mass create is not.
constexpr uint64_t kRegIpWindowMs = 900'000; // 15 min
constexpr uint32_t kRegIpMax = 8;

// Resume uses a softer IP budget so auto-login / reconnect stays smooth.
constexpr uint64_t kResumeIpWindowMs = 60'000;
constexpr uint32_t kResumeIpMax = 60;

constexpr uint64_t kPruneEveryMs = 30'000;
constexpr size_t kMaxMapEntries = 4096;

uint64_t now_ms() {
    using clock = std::chrono::steady_clock;
    return (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        clock::now().time_since_epoch()).count();
}

struct ConnCount {
    uint32_t n = 0;
};

struct WindowCounter {
    uint64_t window_start = 0;
    uint32_t count = 0;
};

struct FailState {
    uint32_t fails = 0;
    uint64_t locked_until = 0;
    uint64_t last_ms = 0;
};

std::unordered_map<std::string, ConnCount> g_conn;
std::unordered_map<std::string, WindowCounter> g_auth_ip;
std::unordered_map<std::string, WindowCounter> g_reg_ip;
std::unordered_map<std::string, WindowCounter> g_resume_ip;
std::unordered_map<std::string, FailState> g_user_fail;
uint64_t g_last_prune = 0;

uint8_t hit_window(WindowCounter &w, uint64_t now, uint64_t window_ms, uint32_t max) {
    if (now - w.window_start >= window_ms) {
        w.window_start = now;
        w.count = 0;
    }
    if (w.count >= max) return 0;
    ++w.count;
    return 1;
}

void prune_windows(std::unordered_map<std::string, WindowCounter> &m, uint64_t now, uint64_t ttl) {
    for (auto it = m.begin(); it != m.end();) {
        if (now - it->second.window_start >= ttl)
            it = m.erase(it);
        else
            ++it;
    }
}

void maybe_prune(uint64_t now) {
    if (now - g_last_prune < kPruneEveryMs) return;
    g_last_prune = now;
    prune_windows(g_auth_ip, now, kAuthIpWindowMs * 2);
    prune_windows(g_reg_ip, now, kRegIpWindowMs * 2);
    prune_windows(g_resume_ip, now, kResumeIpWindowMs * 2);
    for (auto it = g_user_fail.begin(); it != g_user_fail.end();) {
        FailState const &f = it->second;
        uint8_t stale = (f.locked_until == 0 || now >= f.locked_until)
            && (now - f.last_ms >= kFailLockMs * 4);
        if (stale)
            it = g_user_fail.erase(it);
        else
            ++it;
    }
    // Soft cap: drop idle zero-conn keys if map grows (shouldn't normally).
    if (g_conn.size() > kMaxMapEntries) {
        for (auto it = g_conn.begin(); it != g_conn.end();) {
            if (it->second.n == 0)
                it = g_conn.erase(it);
            else
                ++it;
        }
    }
}

} // namespace

void RateLimit::tick_prune() {
    maybe_prune(now_ms());
}

uint8_t RateLimit::allow_connect(std::string const &ip) {
    if (ip.empty()) return 1;
    auto it = g_conn.find(ip);
    if (it == g_conn.end()) return 1;
    return it->second.n < kMaxConnPerIp ? 1 : 0;
}

void RateLimit::on_connect(std::string const &ip) {
    if (ip.empty()) return;
    ++g_conn[ip].n;
}

void RateLimit::on_disconnect(std::string const &ip) {
    if (ip.empty()) return;
    auto it = g_conn.find(ip);
    if (it == g_conn.end()) return;
    if (it->second.n <= 1)
        g_conn.erase(it);
    else
        --it->second.n;
}

uint8_t RateLimit::precheck_auth(std::string const &ip, std::string const &username, uint8_t mode) {
    uint64_t now = now_ms();
    maybe_prune(now);

    // Per-username soft lock after repeated wrong passwords (login only path fails).
    if (!username.empty()) {
        auto uit = g_user_fail.find(username);
        if (uit != g_user_fail.end() && uit->second.locked_until > now)
            return AccountResultCode::kAccountRateLimited;
    }

    if (ip.empty()) return AccountResultCode::kAccountOk;

    if (mode == AccountAuthMode::kAccountResume) {
        WindowCounter &w = g_resume_ip[ip];
        if (!hit_window(w, now, kResumeIpWindowMs, kResumeIpMax))
            return AccountResultCode::kAccountRateLimited;
        return AccountResultCode::kAccountOk;
    }

    WindowCounter &aw = g_auth_ip[ip];
    if (!hit_window(aw, now, kAuthIpWindowMs, kAuthIpMax))
        return AccountResultCode::kAccountRateLimited;

    if (mode == AccountAuthMode::kAccountRegister) {
        WindowCounter &rw = g_reg_ip[ip];
        if (!hit_window(rw, now, kRegIpWindowMs, kRegIpMax))
            return AccountResultCode::kAccountRateLimited;
    }
    return AccountResultCode::kAccountOk;
}

void RateLimit::note_auth_fail(std::string const &ip, std::string const &username, uint8_t mode) {
    (void) ip;
    // Resume token miss / register "exists" are not password guesses.
    if (mode != AccountAuthMode::kAccountLogin) return;
    if (username.empty()) return;
    uint64_t now = now_ms();
    FailState &f = g_user_fail[username];
    f.last_ms = now;
    if (f.locked_until > now) return;
    ++f.fails;
    if (f.fails >= kFailLockCount) {
        f.fails = 0;
        f.locked_until = now + kFailLockMs;
    }
}

void RateLimit::note_auth_ok(std::string const &ip, std::string const &username) {
    (void) ip;
    if (username.empty()) return;
    g_user_fail.erase(username);
}
