#pragma once

#include <cstdint>
#include <string>

// Optional cmake -D overrides (deploy.sh sets VERSION_HASH / SERVER_PORT / WS_URL)
#ifndef VERSION_HASH
extern uint64_t const VERSION_HASH;
#endif

#ifndef SERVER_PORT
extern uint32_t const SERVER_PORT;
#endif

#ifndef WS_URL
extern std::string const WS_URL;
#endif

extern uint32_t const MAX_NAME_LENGTH;
extern uint32_t const MAX_USERNAME_LENGTH;
extern uint32_t const MAX_PASSWORD_LENGTH;
extern uint32_t const MAX_CHAT_LENGTH;
