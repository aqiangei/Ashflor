#pragma once

#include <Shared/Binary.hh>

#include <cstdint>
#include <string>
#include <vector>

class Client;
class GameInstance;

namespace Chat {
    constexpr float NEARBY_RADIUS = 1200.0f;
    constexpr uint32_t LIVE_BROADCAST_CAP = 40;

    struct LiveBroadcast {
        uint32_t id = 0;
        std::string sender;
        std::string text;
    };

    // whisper_to: recipient name for private lines (empty for other channels)
    void send_to(Client *, uint8_t channel, std::string const &sender, std::string const &text,
                 std::string const &whisper_to = {});
    // System 全服（跨地图），不受玩家全服频道冷却限制。
    // whisper_to: optional client hint (e.g. "craft:7" for rarity-colored craft lines)
    void broadcast_server(std::string const &sender, std::string const &text,
                          std::string const &whisper_to = {});
    // Same as broadcast_server, but clients show the aqa-style top banner
    void broadcast_server_banner(std::string const &sender, std::string const &text);
    void handle(Client *, uint8_t channel, std::string const &target, std::string const &text);
    // Newest-first snapshot for /api/live (cap LIVE_BROADCAST_CAP)
    void recent_broadcasts(std::vector<LiveBroadcast> &out);
};
