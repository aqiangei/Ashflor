#pragma once

#include <cstdint>
#include <string>

class Client;
class GameInstance;

namespace Guild {
    constexpr uint32_t MAX_SIZE = 16;
    constexpr uint32_t kNameMax = 12;
    constexpr uint32_t INVITE_TTL_TICKS = 20 * 30; // 30s at 20 TPS

    void leave(Client *);
    uint8_t create(Client *, std::string const &name);
    uint8_t invite(Client *from, std::string const &target_name);
    uint8_t respond(Client *target, std::string const &from_name, uint8_t accept);
    void send_update(Client *);
    void notify(Client *, std::string const &);
    uint8_t same_guild(Client *, Client *);
    void on_login(Client *); // rejoin by account guild_name
    void tick(GameInstance *);
    void broadcast_chat(Client *from, std::string const &sender, std::string const &text);
}
