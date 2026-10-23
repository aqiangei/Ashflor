#include <Server/AdminGiveLog.hh>

#include <chrono>
#include <cstdio>
#include <string>

static char const *GIVE_LOG_FILE = "admin_gives.jsonl";

static void json_escape_append(std::string &out, std::string const &s) {
    for (unsigned char c : s) {
        if (c == '"' || c == '\\') {
            out += '\\';
            out += (char) c;
        } else if (c < 0x20) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned) c);
            out += buf;
        } else {
            out += (char) c;
        }
    }
}

void AdminGiveLog::record(std::string const &account, std::string const &who,
                          uint32_t petal_id, uint8_t rarity, uint32_t count) {
    if (count == 0 || petal_id == 0) return;
    uint64_t ts = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::string line = "{\"ts\":";
    line += std::to_string(ts);
    line += ",\"account\":\"";
    json_escape_append(line, account);
    line += "\",\"who\":\"";
    json_escape_append(line, who);
    line += "\",\"petal_id\":";
    line += std::to_string(petal_id);
    line += ",\"rarity\":";
    line += std::to_string((unsigned) rarity);
    line += ",\"count\":";
    line += std::to_string(count);
    line += "}\n";

    FILE *f = std::fopen(GIVE_LOG_FILE, "ab");
    if (f == nullptr) {
        std::fprintf(stderr, "AdminGiveLog: cannot open %s\n", GIVE_LOG_FILE);
        return;
    }
    std::fwrite(line.data(), 1, line.size(), f);
    std::fflush(f);
    std::fclose(f);
}
