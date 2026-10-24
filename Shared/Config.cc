#include <Shared/Config.hh>

#ifndef VERSION_HASH
extern const uint64_t VERSION_HASH = 19235684322479ull;
#endif

#ifndef SERVER_PORT
extern const uint32_t SERVER_PORT = 9001;
#endif

extern const uint32_t MAX_NAME_LENGTH = 16;
extern const uint32_t MAX_USERNAME_LENGTH = 16;
extern const uint32_t MAX_PASSWORD_LENGTH = 32;
// Room for /w Name message and Chinese guild commands on mobile
extern const uint32_t MAX_CHAT_LENGTH = 96;

#ifndef WS_URL
// Fallback when page cannot derive host (file:// / odd embeds). Prefer location.host at runtime.
extern std::string const WS_URL = "ws://localhost:"+std::to_string(SERVER_PORT);
#endif
