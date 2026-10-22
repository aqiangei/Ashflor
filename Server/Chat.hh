#pragma once

#include <Shared/Binary.hh>

#include <cstdint>
#include <string>
#include <vector>

class Client;
class GameInstance;

namespace Chat {
    constexpr float NEARBY_RADIUS = 1200.0f;
    // Recent slice for /api/live poll
    constexpr uint32_t LIVE_BROADCAST_FEED = 80;
    // Disk + memory caps (×10). Over broadcast cap: drop kill then craft only —
    // announce + chat are never evicted.
    constexpr uint32_t LIVE_BROADCAST_CAP = 80000;
    constexpr uint32_t CHAT_LOG_CAP = 200000;
    // map_id == 255 → 来源未知（后台公告 / 旧数据）。地图字段只表示「在哪发的」，
    // 与是否全服可见无关（全服可见看 kind/channel）。
    constexpr uint8_t MAP_UNKNOWN = 255;
    // 兼容旧名
    constexpr uint8_t MAP_ALL = MAP_UNKNOWN;

    // Eviction when over cap: kill first, then craft. Announce never dropped.
    enum LiveBcKind : uint8_t {
        kKill = 0,
        kCraft = 1,
        kAnnounce = 2
    };

    struct LiveBroadcast {
        uint32_t id = 0;
        uint8_t kind = kAnnounce;
        uint8_t map_id = MAP_ALL;
        uint64_t ts_ms = 0;
        std::string sender;
        std::string player; // subject for filter (crafter / killer / announcer)
        std::string text;
    };

    struct ChatLogEntry {
        uint32_t id = 0;
        uint8_t channel = 0;
        uint8_t map_id = MAP_ALL;
        uint64_t ts_ms = 0;
        std::string from;
        std::string to; // whisper target; else empty
        std::string text;
    };

    void load();
    void flush();

    // whisper_to: recipient name for private lines (empty for other channels)
    void send_to(Client *, uint8_t channel, std::string const &sender, std::string const &text,
                 std::string const &whisper_to = {});
    // System 全服（跨地图）。kind tags live-dashboard archive.
    void broadcast_server(std::string const &sender, std::string const &text,
                          std::string const &whisper_to = {},
                          LiveBcKind kind = kCraft,
                          std::string const &player = {},
                          uint8_t map_id = MAP_ALL);
    // Same as broadcast_server, but clients show the aqa-style top banner
    void broadcast_server_banner(std::string const &sender, std::string const &text,
                                 std::string const &player = {},
                                 uint8_t map_id = MAP_ALL);
    void handle(Client *, uint8_t channel, std::string const &target, std::string const &text);

    // Oldest→newest recent slice for /api/live
    void recent_broadcasts(std::vector<LiveBroadcast> &out);
    // Page older history: before_id=0 means latest page; else entries with id < before_id
    std::string query_broadcasts_json(uint32_t before_id, uint32_t limit,
                                      std::string const &day, std::string const &player,
                                      int kind_filter, int map_filter);
    std::string query_chatlog_json(uint32_t before_id, uint32_t limit,
                                   std::string const &day, std::string const &player,
                                   int channel_filter, int map_filter);

    void log_chat(uint8_t channel, std::string const &from, std::string const &to,
                  std::string const &text, uint8_t map_id = MAP_ALL);
};
