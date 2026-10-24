#pragma once

#include <cstdint>
#include <string>

namespace FeedbackStore {
    void load();
    // Public board; author identity comes from the authenticated game session.
    uint8_t post(std::string const &author, std::string const &text, uint32_t &id,
                 std::string const &source = {});
    std::string page_json(uint32_t before = 0);
#ifdef ASHFLOR_FEEDBACK_TEST
    void set_test_time(uint64_t);
#endif
}
