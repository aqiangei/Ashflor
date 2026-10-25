#pragma once

#include <cstdint>
#include <string>

extern uint8_t INCOMING_PACKET[64 * 1024];
// Must fit kGuestProgress (full craft_bank ≈ kNumPetals varints) + levels.
extern uint8_t OUTGOING_PACKET[64 * 1024];

class Socket {
public:
    uint8_t ready = 0;
    Socket();
    void connect(std::string const);
    void disconnect();
    void send(uint8_t *, uint32_t);
};