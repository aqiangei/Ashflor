#pragma once

#include <string>

// Implemented in Codex.cc — also declared on Server::live_codex_json.
namespace Server {
    std::string live_codex_json();
    // Lightweight maps + mechanics only (for live.html gate tiles).
    std::string live_codex_gate_json();
}
