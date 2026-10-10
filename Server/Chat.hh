#pragma once

#include <Shared/Binary.hh>

#include <string>

class Client;
class GameInstance;

namespace Chat {
    constexpr float NEARBY_RADIUS = 1200.0f;

    void send_to(Client *, uint8_t channel, std::string const &sender, std::string const &text);
    void handle(Client *, uint8_t channel, std::string const &target, std::string const &text);
};
