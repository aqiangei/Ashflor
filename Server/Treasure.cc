#include <Server/Treasure.hh>

#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/TreasureSites.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>

#include <cmath>
#include <cstring>
#include <string>

namespace {

constexpr float kPadSnap = 55.f;
constexpr float kNpcHintR = 280.f;
constexpr float kSiteClaimR = 520.f;
constexpr float kChestOpenR = 70.f;
uint32_t const kHintCooldownTicks = 12 * TPS;

struct SiteRuntime {
    TreasureSites::Kind kind = TreasureSites::kRitual;
    EntityID npc = NULL_ENTITY;
    EntityID pads[2] = { NULL_ENTITY, NULL_ENTITY };
    EntityID crates[2] = { NULL_ENTITY, NULL_ENTITY };
    EntityID chest = NULL_ENTITY; // ChestOnly / ritual roadside
    float cx = 0, cy = 0;
    uint8_t ritual_idx = 0; // 0..1 for claim bit
    uint8_t chest_idx = 255; // 0..7 for claim bit; 255 = none
    uint8_t spawned = 0;
    PetalID::T reward = PetalID::kNone;
    uint16_t reward_n = 0;
    uint32_t coins = 0;
    char const *name_zh = "";
    char const *name_en = "";
    char const *hint_zh = "";
    char const *hint_en = "";
};

struct MapRuntime {
    SiteRuntime sites[TreasureSites::kMaxSitesPerMap];
    uint8_t site_count = 0;
    uint8_t spawned = 0;
};

static MapRuntime g_maps[MapID::kNumMaps];

static void place_xy(uint8_t map_id, float &x, float &y) {
    Map::resolve_circle_terrain(map_id, x, y, 40.f);
    x = fclamp(x, 200.f, (float) ARENA_WIDTH - 200.f);
    y = fclamp(y, 200.f, (float) ARENA_HEIGHT - 200.f);
}

static Entity &spawn_prop(Simulation *sim, MobID::T id, float x, float y,
                          char const *name_override = nullptr) {
    Entity &m = alloc_mob(sim, id, x, y, NULL_ENTITY, [&](Entity &ent) {
        BitMath::set(ent.flags, EntityFlags::kNoDrops);
        BitMath::set(ent.flags, EntityFlags::kHasCulling);
        ent.score_reward = 0;
        ent.damage = 0;
        if (id == MobID::kPushCrate) {
            ent.mass = 3.2f;
            ent.friction = DEFAULT_FRICTION * 2.4f;
            ent.health = ent.max_health = 500000.f;
        } else if (id == MobID::kYahahaPad) {
            ent.mass = 10000.f;
            ent.friction = 1.f;
            ent.health = ent.max_health = 500000.f;
            BitMath::set(ent.flags, EntityFlags::kNoFriendlyCollision);
        } else if (id == MobID::kTreasureNpc) {
            ent.mass = 10000.f;
            ent.health = ent.max_health = 500000.f;
            ent.set_nametag_visible(1);
        } else if (id == MobID::kLootChest) {
            ent.mass = 10000.f;
            ent.health = ent.max_health = 500000.f;
            BitMath::set(ent.flags, EntityFlags::kNoFriendlyCollision);
            ent.set_nametag_visible(0);
        }
        if (name_override != nullptr && name_override[0] != '\0')
            ent.set_name(name_override);
    });
    return m;
}

static uint8_t pad_occupied(Simulation *sim, EntityID pad_id, EntityID const crates[2]) {
    if (!sim->ent_alive(pad_id)) return 0;
    Entity &pad = sim->get_ent(pad_id);
    for (uint8_t i = 0; i < 2; ++i) {
        if (!sim->ent_alive(crates[i])) continue;
        Entity &c = sim->get_ent(crates[i]);
        float dx = c.get_x() - pad.get_x();
        float dy = c.get_y() - pad.get_y();
        if (dx * dx + dy * dy <= kPadSnap * kPadSnap) return 1;
    }
    return 0;
}

static uint8_t puzzle_done(Simulation *sim, SiteRuntime const &site) {
    if (site.kind != TreasureSites::kRitual) return 0;
    return pad_occupied(sim, site.pads[0], site.crates)
        && pad_occupied(sim, site.pads[1], site.crates);
}

static void soft_snap_crates(Simulation *sim, SiteRuntime &rt) {
    if (rt.kind != TreasureSites::kRitual) return;
    uint8_t crate_taken[2] = { 0, 0 };
    for (uint8_t p = 0; p < 2; ++p) {
        if (!sim->ent_alive(rt.pads[p])) continue;
        Entity &pad = sim->get_ent(rt.pads[p]);
        int best = -1;
        float best_d2 = kPadSnap * kPadSnap;
        for (uint8_t c = 0; c < 2; ++c) {
            if (crate_taken[c] || !sim->ent_alive(rt.crates[c])) continue;
            Entity &crate = sim->get_ent(rt.crates[c]);
            float dx = pad.get_x() - crate.get_x();
            float dy = pad.get_y() - crate.get_y();
            float d2 = dx * dx + dy * dy;
            if (d2 < best_d2) { best_d2 = d2; best = (int) c; }
        }
        if (best < 0) continue;
        crate_taken[best] = 1;
        Entity &crate = sim->get_ent(rt.crates[best]);
        float dx = pad.get_x() - crate.get_x();
        float dy = pad.get_y() - crate.get_y();
        if (best_d2 > 1.f) {
            crate.set_x(crate.get_x() + dx * 0.10f);
            crate.set_y(crate.get_y() + dy * 0.10f);
            crate.velocity *= 0.8f;
        }
    }
}

} // namespace

void Treasure::spawn_for_map(GameInstance *game) {
    if (game == nullptr) return;
    uint8_t mid = game->map_id;
    if (mid >= MapID::kNumMaps) return;
    MapRuntime &mr = g_maps[mid];
    if (mr.spawned) return;

    TreasureSites::MapPack pack = TreasureSites::pack(mid);
    Simulation *sim = &game->simulation;
    uint8_t ritual_n = 0;
    uint8_t chest_n = 0;

    mr.site_count = 0;
    for (uint8_t i = 0; i < pack.site_count && mr.site_count < TreasureSites::kMaxSitesPerMap; ++i) {
        TreasureSites::SiteDef const &lay = pack.sites[i];
        SiteRuntime &rt = mr.sites[mr.site_count];
        rt = SiteRuntime{};
        rt.kind = lay.kind;
        rt.name_zh = lay.name_zh;
        rt.name_en = lay.name_en;
        rt.hint_zh = lay.hint_zh;
        rt.hint_en = lay.hint_en;
        rt.reward = lay.reward;
        rt.reward_n = lay.reward_n;
        rt.coins = lay.coins;

        float cx = MAP_SPAWN_X + lay.ox;
        float cy = MAP_SPAWN_Y + lay.oy;
        place_xy(mid, cx, cy);
        rt.cx = cx;
        rt.cy = cy;

        if (lay.kind == TreasureSites::kRitual) {
            rt.ritual_idx = ritual_n++;
            float nx = cx - 40.f, ny = cy - 120.f;
            place_xy(mid, nx, ny);
            rt.npc = spawn_prop(sim, MobID::kTreasureNpc, nx, ny, lay.name_zh).id;
            for (uint8_t p = 0; p < 2; ++p) {
                float px = cx + lay.pad[p][0], py = cy + lay.pad[p][1];
                place_xy(mid, px, py);
                rt.pads[p] = spawn_prop(sim, MobID::kYahahaPad, px, py).id;
                float bx = cx + lay.crate[p][0], by = cy + lay.crate[p][1];
                place_xy(mid, bx, by);
                rt.crates[p] = spawn_prop(sim, MobID::kPushCrate, bx, by).id;
            }
            // One roadside chest near ritual
            if (chest_n < TreasureSites::kMaxChestsPerMap) {
                float hx = cx + 220.f, hy = cy - 160.f;
                place_xy(mid, hx, hy);
                rt.chest = spawn_prop(sim, MobID::kLootChest, hx, hy, "宝箱").id;
                rt.chest_idx = chest_n++;
            }
        } else if (lay.kind == TreasureSites::kGuide) {
            float nx = cx, ny = cy;
            place_xy(mid, nx, ny);
            rt.npc = spawn_prop(sim, MobID::kTreasureNpc, nx, ny, lay.name_zh).id;
            rt.cx = nx;
            rt.cy = ny;
        } else if (lay.kind == TreasureSites::kChestOnly) {
            if (chest_n >= TreasureSites::kMaxChestsPerMap) continue;
            float hx = cx, hy = cy;
            place_xy(mid, hx, hy);
            rt.chest = spawn_prop(sim, MobID::kLootChest, hx, hy, "宝箱").id;
            rt.chest_idx = chest_n++;
            rt.cx = hx;
            rt.cy = hy;
        }

        rt.spawned = 1;
        ++mr.site_count;
    }
    mr.spawned = 1;
}

void Treasure::tick(GameInstance *game) {
    if (game == nullptr) return;
    uint8_t mid = game->map_id;
    if (mid >= MapID::kNumMaps) return;
    MapRuntime &mr = g_maps[mid];
    if (!mr.spawned) return;
    Simulation *sim = &game->simulation;

    // Only soft-snap rituals that have a living player nearby (not all sites × always).
    constexpr float kSnapInterestR = 700.f;
    constexpr float kSnapInterestR2 = kSnapInterestR * kSnapInterestR;
    uint8_t snap_need[TreasureSites::kMaxSitesPerMap] = {};
    for (Client *client : game->clients) {
        if (client == nullptr || !client->alive()) continue;
        if (!sim->ent_exists(client->camera)) continue;
        Entity &cam = sim->get_ent(client->camera);
        if (!sim->ent_alive(cam.get_player())) continue;
        Entity &player = sim->get_ent(cam.get_player());
        float px = player.get_x(), py = player.get_y();
        for (uint8_t si = 0; si < mr.site_count; ++si) {
            if (mr.sites[si].kind != TreasureSites::kRitual) continue;
            float dx = px - mr.sites[si].cx, dy = py - mr.sites[si].cy;
            if (dx * dx + dy * dy <= kSnapInterestR2) snap_need[si] = 1;
        }
    }
    for (uint8_t si = 0; si < mr.site_count; ++si)
        if (snap_need[si]) soft_snap_crates(sim, mr.sites[si]);

    constexpr float kInterestR = 900.f;
    constexpr float kInterestR2 = kInterestR * kInterestR;

    for (Client *client : game->clients) {
        if (client == nullptr || !client->alive()) continue;
        if (!sim->ent_exists(client->camera)) continue;
        Entity &cam = sim->get_ent(client->camera);
        if (!sim->ent_alive(cam.get_player())) continue;
        Entity &player = sim->get_ent(cam.get_player());
        float px = player.get_x(), py = player.get_y();

        if (client->treasure_hint_cd > 0) --client->treasure_hint_cd;

        for (uint8_t si = 0; si < mr.site_count; ++si) {
            SiteRuntime &rt = mr.sites[si];
            if (!rt.spawned) continue;
            // Cheap center cull before any ent lookups
            float sdx = px - rt.cx, sdy = py - rt.cy;
            float sd2 = sdx * sdx + sdy * sdy;
            if (sd2 > kInterestR2) continue;

            if (sim->ent_alive(rt.npc) && client->treasure_hint_cd == 0
                && rt.hint_zh != nullptr && rt.hint_zh[0] != '\0') {
                Entity &npc = sim->get_ent(rt.npc);
                float ndx = px - npc.get_x();
                float ndy = py - npc.get_y();
                if (ndx * ndx + ndy * ndy <= kNpcHintR * kNpcHintR) {
                    Chat::send_to(client, ChatChannel::kChatSystem, "",
                        std::string{"["} + rt.name_zh + "] " + rt.hint_zh);
                    Chat::send_to(client, ChatChannel::kChatSystem, "",
                        std::string{"["} + rt.name_en + "] " + rt.hint_en);
                    client->treasure_hint_cd = kHintCooldownTicks;
                }
            }

            if (sim->ent_alive(rt.chest) && rt.chest_idx < TreasureSites::kMaxChestsPerMap) {
                uint8_t bit = (uint8_t) (1u << rt.chest_idx);
                if (!(client->treasure_chest_claimed[mid] & bit)) {
                    Entity &chest = sim->get_ent(rt.chest);
                    float cdx = px - chest.get_x();
                    float cdy = py - chest.get_y();
                    if (cdx * cdx + cdy * cdy <= kChestOpenR * kChestOpenR) {
                        client->treasure_chest_claimed[mid] |= bit;
                        // Sparse population → generous chest loot (existing channels only)
                        uint32_t coins = 220 + mid * 55 + rt.chest_idx * 70;
                        client_award_coins(client, coins);
                        std::string chest_title =
                            std::string{"神秘宝箱 · "} + (rt.name_zh ? rt.name_zh : "");
                        client_send_loot_toast(client, PetalID::kCoin, (uint16_t) coins,
                            chest_title);
                        // Sensory pile: 5–8 petals, Ultra/Super/Unique full pool.
                        uint8_t mat_n = 5 + (((uint32_t) (px + py) + mid * 17
                            + rt.chest_idx * 31) % 4); // 5–8
                        uint8_t mat_tier = (rt.chest_idx >= 2)
                            ? TreasureSites::kTierRitual : TreasureSites::kTierHard;
                        static char const *const kRarityZh[RarityID::kNumRarities] = {
                            "普通", "罕见", "稀有", "史诗", "传奇", "神话", "究极", "超凡", "唯一"
                        };
                        uint8_t given_mat = 0;
                        std::string mat_line;
                        for (uint8_t i = 0; i < mat_n; ++i) {
                            PetalID::T mat = TreasureSites::pick_explore_mat(mid, mat_tier);
                            if (mat == PetalID::kNone) continue;
                            if (!client_award_petal(client, mat)) continue;
                            ++given_mat;
                            client_send_loot_toast(client, mat, 1, chest_title);
                            char const *pname = (mat < PetalID::kNumPetals && PETAL_DATA[mat].name)
                                ? PETAL_DATA[mat].name : "残片";
                            if (!mat_line.empty()) mat_line += "、";
                            mat_line += pname;
                            if (mat < PetalID::kNumPetals) {
                                uint8_t rr = PETAL_DATA[mat].rarity;
                                if (rr < RarityID::kNumRarities) {
                                    mat_line += "(";
                                    mat_line += kRarityZh[rr];
                                    mat_line += ")";
                                }
                            }
                        }
                        Chat::send_to(client, ChatChannel::kChatSystem, "",
                            std::string{"【宝箱】钱币 +"} + std::to_string(coins)
                            + (given_mat ? "，花瓣 ×" + std::to_string(given_mat)
                                + "：" + mat_line : ""));
                    }
                }
            }

            if (rt.kind != TreasureSites::kRitual) continue;
            if (sd2 > kSiteClaimR * kSiteClaimR) continue;
            if (rt.ritual_idx >= TreasureSites::kMaxRitualsPerMap) continue;
            uint8_t rbit = (uint8_t) (1u << rt.ritual_idx);
            if (client->treasure_ritual_claimed[mid] & rbit) continue;
            if (!puzzle_done(sim, rt)) continue;

            client->treasure_ritual_claimed[mid] |= rbit;
            uint16_t given = 0;
            PetalID::T last_mat = PetalID::kNone;
            // Thematic rolls + generous full-pool bonus so ritual feels like a haul.
            uint16_t thematic_n = rt.reward_n;
            if (thematic_n < 3) thematic_n = 3;
            if (rt.reward != PetalID::kNone) {
                for (uint16_t n = 0; n < thematic_n; ++n) {
                    PetalID::T mat = TreasureSites::pick_ritual_mat(rt.reward);
                    if (mat == PetalID::kNone) mat = rt.reward;
                    if (!client_award_petal(client, mat)) continue;
                    ++given;
                    last_mat = mat;
                }
            }
            for (uint8_t b = 0; b < 4; ++b) {
                PetalID::T bonus = TreasureSites::pick_bonus_pool_petal(
                    TreasureSites::kTierRitual);
                if (bonus == PetalID::kNone) continue;
                if (!client_award_petal(client, bonus)) continue;
                ++given;
                last_mat = bonus;
            }
            uint32_t coins = rt.coins + (rt.coins / 2) + 120;
            if (coins > 0) client_award_coins(client, coins);

            std::string title = std::string{"宝藏 · "} + rt.name_zh;
            if (last_mat != PetalID::kNone)
                client_send_loot_toast(client, last_mat, given, title);
            else
                client_send_loot_toast(client, PetalID::kCoin, (uint16_t) coins, title);

            char const *rname = (last_mat != PetalID::kNone
                    && last_mat < PetalID::kNumPetals && PETAL_DATA[last_mat].name)
                ? PETAL_DATA[last_mat].name
                : ((rt.reward < PetalID::kNumPetals && PETAL_DATA[rt.reward].name)
                    ? PETAL_DATA[rt.reward].name : "?");
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                std::string{"【宝藏】完成推箱："}
                + rname + " ×" + std::to_string(given)
                + "（含稀有度阶梯），钱币 +" + std::to_string(coins));
        }
    }
}
