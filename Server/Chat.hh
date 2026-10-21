#pragma once

#include <Shared/Binary.hh>

#include <string>

class Client;
class GameInstance;

namespace Chat {
    constexpr float NEARBY_RADIUS = 1200.0f;

    // whisper_to: recipient name for private lines (empty for other channels)
    void send_to(Client *, uint8_t channel, std::string const &sender, std::string const &text,
                 std::string const &whisper_to = {});
    // System 全服（跨地图），不受玩家全服频道冷却限制
    void broadcast_server(std::string const &sender, std::string const &text);
    void handle(Client *, uint8_t channel, std::string const &target, std::string const &text);
};
