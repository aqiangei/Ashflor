#pragma once

#include <cstdint>
#include <string>

namespace FeedbackStore {
    void load();
    // Public board; author identity comes from the authenticated game session.
    uint8_t post(std::string const &author, std::string const &text, uint32_t &id,
                 std::string const &source = {});
    std::string page_json(uint32_t before = 0);
    // Moderation: key must match hardcoded ops secret. Returns JSON {ok,msg,id,handled}.
    std::string set_handled_json(uint32_t id, uint8_t handled, std::string const &key);
    std::string auth_json(std::string const &key);
    // Author marks whether their issue is resolved (authenticated account name).
    uint8_t set_resolved(std::string const &author, uint32_t id, uint8_t resolved);
#ifdef ASHFLOR_FEEDBACK_TEST
    void set_test_time(uint64_t);
#endif
}
