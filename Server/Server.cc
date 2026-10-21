#include <Server/Server.hh>

#include <Server/AccountStore.hh>
#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Guild.hh>
#include <Server/LeaderboardStore.hh>
#include <Server/RateLimit.hh>
#include <Server/Spawn.hh>
#include <Server/XpMigration.hh>

#include <Helpers/Math.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>
#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <cstring>

#include <Shared/PetalNameZh.inc>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <new>
#include <string>
#include <vector>

static bool was_draining = false;

namespace Server {
    uint8_t OUTGOING_PACKET[MAX_PACKET_LEN] = {0};
    GameInstance *games = nullptr;
    volatile sig_atomic_t is_draining = 0;
    bool is_stopping = false;
    uint32_t player_count = 0;
}

using namespace Server;

GameInstance *Server::game_for(Simulation *sim) {
    if (sim == nullptr) return nullptr;
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
        if (&games[i].simulation == sim) return &games[i];
    }
    return nullptr;
}

Client *Server::client_by_camera(Simulation *sim, EntityID const &id) {
    GameInstance *game = game_for(sim);
    if (game == nullptr) return nullptr;
    return game->client_by_camera(id);
}

void Server::tick() {
    using namespace std::chrono_literals;
    auto start = std::chrono::steady_clock::now();
    static uint32_t tick_n = 0;
    ++tick_n;
    if ((tick_n % 75) == 0) // ~3s at 25 TPS
        RateLimit::tick_prune();
    uint32_t online = 0;
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
        // Empty maps: ~1 Hz. Full collide on 8 packed biomes was the tick hog
        // (segmented swarms × radius scale); occupied maps still tick every frame.
        if (Server::games[i].clients.empty() && ((tick_n + i) % 25) != 0) {
            continue;
        }
        Server::games[i].tick();
        for (Client *c : Server::games[i].clients)
            client_tick_bonus(c);
        online += (uint32_t) Server::games[i].clients.size();
    }
    Server::player_count = online;
    // Online roster under minimap (~2s): name, map, guild, level
    if ((tick_n % 50) == 0 && online > 0) {
        struct Row {
            std::string name;
            uint8_t map_id;
            std::string guild;
            uint32_t level;
        };
        std::vector<Row> rows;
        rows.reserve(online);
        for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
            for (Client *c : Server::games[mi].clients) {
                if (c == nullptr || c->play_name.empty()) continue;
                uint32_t level = 1;
                if (c->game != nullptr && c->game->simulation.ent_exists(c->camera)) {
                    Entity &cam = c->game->simulation.get_ent(c->camera);
                    if (c->game->simulation.ent_alive(cam.get_player()))
                        level = score_to_level(c->game->simulation.get_ent(cam.get_player()).get_score());
                    else
                        level = cam.get_respawn_level();
                }
                if (level < 1) level = 1;
                rows.push_back({ c->play_name, mi, c->guild_name, level });
            }
        }
        if (rows.size() > 64) rows.resize(64);
        Writer writer(Server::OUTGOING_PACKET);
        writer.write<uint8_t>(Clientbound::kOnlineRoster);
        writer.write<uint8_t>((uint8_t) rows.size());
        for (Row const &r : rows) {
            writer.write<std::string>(r.name);
            writer.write<uint8_t>(r.map_id);
            writer.write<std::string>(r.guild);
            writer.write<uint32_t>(r.level);
        }
        size_t plen = (size_t) (writer.at - writer.packet);
        for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
            for (Client *c : Server::games[mi].clients) {
                if (c == nullptr) continue;
                c->send_packet(writer.packet, plen);
            }
        }
    }
    static uint32_t lb_flush_ticks = 0;
    if (++lb_flush_ticks >= 25) { // ~1s at 25 TPS
        lb_flush_ticks = 0;
        LeaderboardStore::flush();
        // Play time counter: one second per connected, spawned player
        for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
            for (Client *c : Server::games[i].clients) {
                if (c == nullptr || c->play_name.empty()) continue;
                if (c->stat_playtime < 4000000000u)
                    ++c->stat_playtime;
            }
        }
    }
    // Push the session stat counters into the leaderboard store (~5s)
    static uint32_t stat_flush_ticks = 0;
    if (++stat_flush_ticks >= 125) {
        stat_flush_ticks = 0;
        uint8_t any = 0;
        for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
            for (Client *c : Server::games[i].clients) {
                if (c == nullptr) continue;
                client_flush_stats(c);
                any = 1;
            }
        }
        if (any) LeaderboardStore::flush();
    }
    // Periodic full account snapshot for all logged-in clients (~5s)
    static uint32_t acc_flush_ticks = 0;
    if (++acc_flush_ticks >= 125) {
        acc_flush_ticks = 0;
        AccountStore::prune_stale_sessions();
        uint8_t any = 0;
        for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
            for (Client *c : Server::games[i].clients) {
                if (c == nullptr || c->account_name.empty()) continue;
                AccountStore::capture_nosave(c);
                any = 1;
            }
        }
        if (any) AccountStore::save();
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> tick_time = end - start;
    // 20 TPS budget ≈ 50ms; log only when we actually miss the frame
    if (tick_time > 40ms) std::cout << "tick took " << tick_time << '\n';

    if (Server::is_draining && !was_draining) {
        was_draining = true;
        std::cout << "draining...\n";
        AccountStore::save();
        Guild::save();
        LeaderboardStore::flush();
    }
    if (Server::is_draining && !Server::is_stopping && Server::player_count == 0)
        Server::stop();
}

static void json_escape_append(std::string &out, std::string const &s) {
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += (char) c;
                }
                break;
        }
    }
}

static char const *live_rarity_zh(uint8_t rarity) {
    static char const *const names[RarityID::kNumRarities] = {
        "普通", "罕见", "稀有", "史诗", "传奇", "神话", "究极", "超凡", "独特"
    };
    if (rarity >= RarityID::kNumRarities) return "?";
    return names[rarity];
}

static char const *live_lb_category_zh(uint8_t category) {
    switch (category) {
        case kLbPlayTime: return "在线时长";
        case kLbLevel: return "等级";
        case kLbMessages: return "发言";
        case kLbKills: return "Boss击杀";
        case kLbCrafts: return "高级合成";
        default: return "?";
    }
}

static void json_append_petal(std::string &out, PetalID::T id) {
    out += "{\"id\":";
    out += std::to_string((uint32_t) id);
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) {
        out += ",\"name\":\"\",\"rarity\":0,\"rarity_zh\":\"\"}";
        return;
    }
    out += ",\"name\":\"";
    json_escape_append(out, PETAL_DATA[id].name);
    out += "\",\"rarity\":";
    out += std::to_string(PETAL_DATA[id].rarity);
    out += ",\"rarity_zh\":\"";
    out += live_rarity_zh(PETAL_DATA[id].rarity);
    out += "\"}";
}

static void json_append_slot_list(std::string &out, PetalID::T const *slots, uint32_t n, uint8_t skip_empty) {
    out += '[';
    uint8_t first = 1;
    for (uint32_t i = 0; i < n; ++i) {
        if (skip_empty && slots[i] == PetalID::kNone) continue;
        if (!first) out += ',';
        first = 0;
        json_append_petal(out, slots[i]);
    }
    out += ']';
}

std::string Server::live_status_json() {
    static char const *const MAP_ZH[MapID::kNumMaps] = {
        "花园", "沙漠", "海洋", "雪原", "丛林", "蚁穴", "火山", "王城"
    };
    auto now_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    uint32_t map_counts[MapID::kNumMaps] = {};
    std::string out;
    out.reserve(8192);
    out += "{\"online\":";
    out += std::to_string(Server::player_count);
    out += ",\"draining\":";
    out += Server::is_draining ? "true" : "false";
    out += ",\"accounts\":";
    out += std::to_string(LeaderboardStore::recorded_count());
    out += ",\"version_hash\":";
    out += std::to_string(VERSION_HASH);
    out += ",\"ts\":";
    out += std::to_string(now_ms);

    // --- Bosses ---
    out += ",\"bosses\":[";
    if (Server::games != nullptr) {
        for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
            if (mi) out += ',';
            float bx = 0.f, by = 0.f;
            uint8_t mob_id = 0;
            uint8_t has = Map::boss_anchor(mi, bx, by, mob_id);
            Simulation &sim = Server::games[mi].simulation;
            uint8_t alive = 0;
            float hp = 0.f, max_hp = 0.f;
            if (has && mob_id < MobID::kNumMobs) {
                sim.for_each<kMob>([&](Simulation *, Entity &m) {
                    if (alive) return;
                    if (m.get_mob_id() != mob_id || m.pending_delete) return;
                    alive = 1;
                    hp = m.health;
                    max_hp = m.max_health;
                });
            }
            out += "{\"map_id\":";
            out += std::to_string(mi);
            out += ",\"map\":\"";
            out += MAP_ZH[mi];
            out += "\",\"has_boss\":";
            out += has ? "true" : "false";
            out += ",\"mob_id\":";
            out += std::to_string(mob_id);
            out += ",\"name\":\"";
            if (has && mob_id < MobID::kNumMobs)
                json_escape_append(out, MOB_DATA[mob_id].name);
            out += "\",\"alive\":";
            out += alive ? "true" : "false";
            out += ",\"hp\":";
            out += std::to_string((int) (hp + 0.5f));
            out += ",\"max_hp\":";
            out += std::to_string((int) (max_hp + 0.5f));
            out += ",\"respawn_sec\":";
            if (!alive && sim.boss_ever_spawned && sim.boss_respawn_ticks > 0)
                out += std::to_string((sim.boss_respawn_ticks + (int32_t) TPS - 1) / (int32_t) TPS);
            else
                out += "0";
            out += '}';
        }
    }
    out += ']';

    // --- Leaderboard: score + stat categories ---
    {
        std::vector<LeaderboardEntry> score_top;
        LeaderboardStore::fill_top(score_top);
        out += ",\"leaderboard\":{\"score\":[";
        for (size_t i = 0; i < score_top.size(); ++i) {
            if (i) out += ',';
            out += "{\"name\":\"";
            json_escape_append(out, score_top[i].name);
            out += "\",\"score\":";
            out += std::to_string(score_top[i].score);
            out += ",\"level\":";
            out += std::to_string(score_top[i].level);
            out += '}';
        }
        out += "],\"stats\":[";
        for (uint8_t cat = 0; cat < kLbCategoryCount; ++cat) {
            if (cat) out += ',';
            std::vector<LeaderboardStat> top;
            LeaderboardStore::fill_top_stats(top, cat, LEADERBOARD_STAT_SIZE);
            out += "{\"id\":";
            out += std::to_string(cat);
            out += ",\"name\":\"";
            out += live_lb_category_zh(cat);
            out += "\",\"rows\":[";
            for (size_t i = 0; i < top.size(); ++i) {
                if (i) out += ',';
                out += "{\"name\":\"";
                json_escape_append(out, top[i].name);
                out += "\",\"value\":";
                out += std::to_string(top[i].value);
                out += '}';
            }
            out += "]}";
        }
        out += "]}";
    }

    // --- Broadcasts ---
    {
        std::vector<Chat::LiveBroadcast> bcs;
        Chat::recent_broadcasts(bcs);
        out += ",\"broadcasts\":[";
        for (size_t i = 0; i < bcs.size(); ++i) {
            if (i) out += ',';
            out += "{\"id\":";
            out += std::to_string(bcs[i].id);
            out += ",\"sender\":\"";
            json_escape_append(out, bcs[i].sender);
            out += "\",\"text\":\"";
            json_escape_append(out, bcs[i].text);
            out += "\"}";
        }
        out += ']';
    }

    // --- Maps + players (with inventory) ---
    out += ",\"maps\":[";
    std::string players_json = "[";
    uint8_t first_player = 1;
    if (Server::games != nullptr) {
        for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
            map_counts[mi] = (uint32_t) Server::games[mi].clients.size();
            if (mi) out += ',';
            out += "{\"id\":";
            out += std::to_string(mi);
            out += ",\"name\":\"";
            out += MAP_ZH[mi];
            out += "\",\"players\":";
            out += std::to_string(map_counts[mi]);
            out += '}';

            for (Client *c : Server::games[mi].clients) {
                if (c == nullptr) continue;
                uint32_t level = 1;
                uint8_t alive = 0;
                uint8_t in_game = !c->play_name.empty();
                float hp = 0.f, max_hp = 0.f;
                PetalID::T primary[MAX_SLOT_COUNT] = {};
                PetalID::T bag[MAX_SLOT_COUNT] = {};
                uint32_t primary_n = 0;
                uint32_t bag_n = 0;
                if (c->game != nullptr && c->game->simulation.ent_exists(c->camera)) {
                    Simulation *sim = &c->game->simulation;
                    Entity &cam = sim->get_ent(c->camera);
                    Entity *flower = nullptr;
                    if (sim->ent_alive(cam.get_player())) {
                        flower = &sim->get_ent(cam.get_player());
                        level = score_to_level(flower->get_score());
                        alive = 1;
                        in_game = 1;
                        hp = flower->health;
                        max_hp = flower->max_health;
                    } else {
                        level = cam.get_respawn_level();
                    }
                    if (level < 1) level = 1;
                    primary_n = flower != nullptr
                        ? flower->get_loadout_count()
                        : Talent::slot_count_from_ranks(c->talent_ranks);
                    if (primary_n > MAX_SLOT_COUNT) primary_n = MAX_SLOT_COUNT;
                    bag_n = MAX_SLOT_COUNT;
                    for (uint32_t i = 0; i < primary_n; ++i) {
                        primary[i] = flower != nullptr
                            ? flower->get_loadout_ids(i)
                            : cam.get_inventory(i);
                    }
                    for (uint32_t i = 0; i < bag_n; ++i) {
                        uint32_t slot = primary_n + i;
                        bag[i] = flower != nullptr
                            ? flower->get_loadout_ids(slot)
                            : cam.get_inventory(slot);
                    }
                }
                if (level < 1) level = 1;

                if (!first_player) players_json += ',';
                first_player = 0;
                players_json += "{\"name\":\"";
                json_escape_append(players_json, in_game ? c->play_name : std::string(""));
                players_json += "\",\"account\":\"";
                json_escape_append(players_json, c->account_name);
                players_json += "\",\"guild\":\"";
                json_escape_append(players_json, c->guild_name);
                players_json += "\",\"map\":\"";
                players_json += MAP_ZH[mi];
                players_json += "\",\"map_id\":";
                players_json += std::to_string(mi);
                players_json += ",\"level\":";
                players_json += std::to_string(level);
                players_json += ",\"alive\":";
                players_json += alive ? "true" : "false";
                players_json += ",\"in_game\":";
                players_json += in_game ? "true" : "false";
                players_json += ",\"coins\":";
                players_json += std::to_string(c->coins);
                players_json += ",\"hp\":";
                players_json += std::to_string((int) (hp + 0.5f));
                players_json += ",\"max_hp\":";
                players_json += std::to_string((int) (max_hp + 0.5f));
                players_json += ",\"loadout\":";
                json_append_slot_list(players_json, primary, primary_n, 0);
                players_json += ",\"bag\":";
                json_append_slot_list(players_json, bag, bag_n, 1);
                players_json += ",\"bank\":[";
                uint8_t first_bank = 1;
                uint32_t bank_n = 0;
                for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals && bank_n < 48; ++id) {
                    uint32_t cnt = c->craft_bank[id];
                    if (cnt == 0) continue;
                    if (!first_bank) players_json += ',';
                    first_bank = 0;
                    ++bank_n;
                    players_json += "{\"count\":";
                    players_json += std::to_string(cnt);
                    players_json += ",\"petal\":";
                    json_append_petal(players_json, (PetalID::T) id);
                    players_json += '}';
                }
                players_json += "]}";
            }
        }
    }
    out += ']';
    players_json += ']';
    out += ",\"players\":";
    out += players_json;
    out += '}';
    return out;
}

static std::string live_admin_ok(std::string const &msg) {
    std::string out = "{\"ok\":true,\"msg\":\"";
    json_escape_append(out, msg);
    out += "\"}";
    return out;
}

static std::string live_admin_err(std::string const &msg) {
    std::string out = "{\"ok\":false,\"msg\":\"";
    json_escape_append(out, msg);
    out += "\"}";
    return out;
}

static Client *live_find_client(std::string const &who) {
    if (who.empty() || Server::games == nullptr) return nullptr;
    Client *by_account = nullptr;
    for (uint8_t mi = 0; mi < MapID::kNumMaps; ++mi) {
        for (Client *c : Server::games[mi].clients) {
            if (c == nullptr) continue;
            if (!c->play_name.empty() && c->play_name == who) return c;
            if (by_account == nullptr && !c->account_name.empty() && c->account_name == who)
                by_account = c;
        }
    }
    return by_account;
}

std::string Server::live_catalog_json() {
    std::string out = "{\"rarities\":[";
    for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
        if (r) out += ',';
        out += "{\"id\":";
        out += std::to_string(r);
        out += ",\"name_zh\":\"";
        out += live_rarity_zh(r);
        out += "\"}";
    }
    out += "],\"petals\":[";
    uint8_t first = 1;
    for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
        if (!first) out += ',';
        first = 0;
        char const *en = PETAL_DATA[id].name;
        out += "{\"id\":";
        out += std::to_string(id);
        out += ",\"name\":\"";
        json_escape_append(out, en ? en : "");
        out += "\",\"name_zh\":\"";
        json_escape_append(out, LIVE_PETAL_NAME_ZH[id] ? LIVE_PETAL_NAME_ZH[id] : "");
        out += "\",\"rarity\":";
        out += std::to_string(PETAL_DATA[id].rarity);
        out += ",\"rarity_zh\":\"";
        out += live_rarity_zh(PETAL_DATA[id].rarity);
        out += "\",\"rarities\":[";
        uint8_t rf = 1;
        for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
            if (Craft::petal_at_rarity((PetalID::T) id, r) == PetalID::kNone) continue;
            if (!rf) out += ',';
            rf = 0;
            out += std::to_string(r);
        }
        out += "]}";
    }
    out += "],\"mobs\":[";
    first = 1;
    for (uint32_t id = 1; id < (uint32_t) MobID::kNumMobs; ++id) {
        if (!first) out += ',';
        first = 0;
        out += "{\"id\":";
        out += std::to_string(id);
        out += ",\"name\":\"";
        json_escape_append(out, MOB_DATA[id].name);
        out += "\",\"rarity\":";
        out += std::to_string(MOB_DATA[id].rarity);
        out += ",\"boss\":";
        out += is_boss_mob((MobID::T) id) ? "true" : "false";
        out += '}';
    }
    out += "]}";
    return out;
}

std::string Server::live_admin_announce(std::string const &text) {
    std::string t = text;
    while (!t.empty() && (t.back() == '\n' || t.back() == '\r' || t.back() == ' '))
        t.pop_back();
    if (t.empty()) return live_admin_err("公告内容为空");
    if (t.size() > 200) return live_admin_err("公告过长（最多200字）");
    // Do NOT spoof as player "aqa" — that looked like the owner typed in nearby chat.
    // Banner flag is whisper_to "banner" (see Chat::broadcast_server_banner).
    Chat::broadcast_server_banner("公告", t);
    return live_admin_ok("已全服公告");
}

std::string Server::live_admin_give(std::string const &who, uint32_t petal_id, uint32_t count,
                                   uint32_t rarity) {
    if (who.empty()) return live_admin_err("未指定玩家");
    if (petal_id == 0 || petal_id >= PetalID::kNumPetals)
        return live_admin_err("无效花瓣 id");
    if (count < 1) count = 1;
    if (count > 64) count = 64;

    PetalID::T give_id = (PetalID::T) petal_id;
    char const *base_en = PETAL_DATA[petal_id].name ? PETAL_DATA[petal_id].name : "?";
    char const *base_zh = LIVE_PETAL_NAME_ZH[petal_id] ? LIVE_PETAL_NAME_ZH[petal_id] : base_en;

    if (rarity != 255) {
        if (rarity >= RarityID::kNumRarities)
            return live_admin_err("无效稀有度等级");
        PetalID::T resolved = Craft::petal_at_rarity((PetalID::T) petal_id, (uint8_t) rarity);
        if (resolved == PetalID::kNone) {
            std::string err = "「";
            err += base_zh;
            err += "」没有【";
            err += live_rarity_zh((uint8_t) rarity);
            err += "】等级。现有：";
            uint8_t any = 0;
            for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
                if (Craft::petal_at_rarity((PetalID::T) petal_id, r) == PetalID::kNone) continue;
                if (any) err += "、";
                any = 1;
                err += live_rarity_zh(r);
            }
            if (!any) err += "无";
            return live_admin_err(err);
        }
        give_id = resolved;
    }

    Client *c = live_find_client(who);
    if (c == nullptr) return live_admin_err("找不到在线玩家：" + who);
    uint32_t given = 0;
    for (uint32_t i = 0; i < count; ++i) {
        if (client_award_petal(c, give_id)) ++given;
        else break;
    }
    if (given == 0) return live_admin_err("发放失败（背包/银行可能已满）");
    client_send_loot_toast(c, give_id, (uint16_t) given, "管理员发放");
    std::string msg = "已向 ";
    msg += who;
    msg += " 发放 ";
    msg += std::to_string(given);
    msg += "× ";
    msg += base_zh;
    msg += "【";
    msg += live_rarity_zh(PETAL_DATA[give_id].rarity);
    msg += "】";
    return live_admin_ok(msg);
}

std::string Server::live_admin_spawn(uint8_t map_id, uint32_t mob_id, float x, float y,
                                    std::string const &near_who) {
    if (Server::games == nullptr) return live_admin_err("服务器未就绪");
    if (map_id >= MapID::kNumMaps) return live_admin_err("无效地图");
    if (mob_id == 0 || mob_id >= MobID::kNumMobs) return live_admin_err("无效怪物 id");
    GameInstance &game = Server::games[map_id];
    Simulation *sim = &game.simulation;
    float sx = x, sy = y;
    if (!near_who.empty()) {
        Client *c = live_find_client(near_who);
        if (c == nullptr) return live_admin_err("找不到参照玩家：" + near_who);
        if (c->game == nullptr || c->game->map_id != map_id)
            return live_admin_err("该玩家不在所选地图");
        if (!c->alive()) return live_admin_err("参照玩家未存活");
        Entity &cam = c->game->simulation.get_ent(c->camera);
        Entity &pl = c->game->simulation.get_ent(cam.get_player());
        sx = pl.get_x() + (frand() - 0.5f) * 400.f;
        sy = pl.get_y() + (frand() - 0.5f) * 400.f;
    } else if (!(sx > 0.f && sy > 0.f && sx < ARENA_WIDTH && sy < ARENA_HEIGHT)) {
        sx = MAP_SPAWN_X + (frand() - 0.5f) * 600.f;
        sy = MAP_SPAWN_Y + (frand() - 0.5f) * 600.f;
    }
    Map::resolve_circle_terrain(map_id, sx, sy, 80.f);
    alloc_mob(sim, (MobID::T) mob_id, sx, sy, NULL_ENTITY);
    std::string msg = "已在";
    static char const *const MAP_ZH[MapID::kNumMaps] = {
        "花园", "沙漠", "海洋", "雪原", "丛林", "蚁穴", "火山", "王城"
    };
    msg += MAP_ZH[map_id];
    msg += " 刷出 ";
    msg += MOB_DATA[mob_id].name;
    msg += " @(";
    msg += std::to_string((int) sx);
    msg += ",";
    msg += std::to_string((int) sy);
    msg += ")";
    return live_admin_ok(msg);
}

void Server::init() {
    AccountStore::load();
    Guild::load();
    LeaderboardStore::load();
    // Correct XP-inflation levels only (safe/gray untouched; no inventory wipe).
    XpMigration::run_once();
    // Seed the ranked "level" column from the saved accounts
    AccountStore::seed_leaderboard_levels();
    LeaderboardStore::flush();
    if (Server::games == nullptr)
        Server::games = new GameInstance[MapID::kNumMaps];
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i)
        Server::games[i].init(i);
    Server::run();
}
