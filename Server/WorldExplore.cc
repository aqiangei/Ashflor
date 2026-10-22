#include <Server/WorldExplore.hh>

#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Spawn.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/TreasureSites.hh>

#include <Helpers/Bits.hh>
#include <Helpers/Math.hh>
#include <Helpers/Vector.hh>

#include <cmath>
#include <cstdint>
#include <string>

namespace {

constexpr float kArriveR = 90.f;
constexpr float kSpiritSpeed = 4.2f; // ~20–40s path across zone
constexpr float kTouchR = 55.f;
constexpr float kMinPathLen = 900.f; // idle↔dest must stay far after terrain snap
// Map-level spirit: roll while any player is on the map (not tied to player pos).
uint32_t const kSpiritCooldownTicks = 60 * TPS; // after arrive
uint32_t const kFailCooldownTicks = 20 * TPS; // after failed roll
uint32_t const kDeathCooldownTicks = 30 * TPS; // if somehow deleted mid-path
uint32_t const kSpawnCheckTicks = 5 * TPS;
constexpr float kSpawnChance = 0.14f;
uint32_t const kMapGraceTicks = 15 * TPS; // first roll after map boot
constexpr float kBossSuppressR2 = 2200.f * 2200.f;
constexpr float kRewardNearR = 520.f; // tip/reward players who followed near dest

static uint8_t near_boss(uint8_t mid, float x, float y) {
    float bx = 0, by = 0;
    uint8_t mob = 0;
    if (!Map::boss_anchor(mid, bx, by, mob)) return 0;
    float dx = x - bx, dy = y - by;
    return (dx * dx + dy * dy) <= kBossSuppressR2;
}

struct MapExploreDef {
    uint8_t map_id;
    float touch_off[3][2];
    float dest_ox, dest_oy;
    float idle_ox, idle_oy; // spawn center
    MobID::T spirit_mob;
    char const *spirit_zh;
    char const *spirit_en;
    char const *arrive_zh;
    char const *arrive_en;
    char const *touch_done_zh;
    char const *touch_done_en;
};

static MapExploreDef const kDefs[] = {
    { MapID::kGarden,
      {{ 1800.f, -600.f }, { 2050.f, -350.f }, { 1650.f, -250.f }},
      2400.f, -1400.f, 1100.f, -420.f, MobID::kBee,
      "迷路小蜜蜂", "Lost Bee",
      "[迷路小蜜蜂] 嗡……推箱子、听路边人说话，残片就会找到你。",
      "[Lost Bee] Buzz… push crates and listen to wayside guides.",
      "【探索】三枚印记按序亮起。花之心甲虫仍在沉睡……",
      "[Explore] Three marks lit. The Flower Guardian still sleeps…" },
    { MapID::kDesert,
      {{ -1600.f, 900.f }, { -1350.f, 1150.f }, { -1850.f, 1200.f }},
      -2200.f, 1800.f, -1050.f, 780.f, MobID::kWanderingGuard,
      "迷路的旅人", "Lost Traveler",
      "[迷路的旅人] 谢了……找沙丘贩子，打听沙虫君主。",
      "[Lost Traveler] Thanks… ask the Dune Dealer about the Monarch.",
      "【探索】三枚沙印按序亮起。沙虫君主在深处呼吸。",
      "[Explore] Three sand marks lit. The Sandworm Monarch breathes below." },
    { MapID::kOcean,
      {{ 1200.f, 1600.f }, { 1450.f, 1850.f }, { 1000.f, 1900.f }},
      1600.f, 2300.f, 980.f, 1180.f, MobID::kMoonJelly,
      "迷路的小水母", "Lost Jelly",
      "[迷路的小水母] ……去找潜水员马尔，深渊巨口还在睡。",
      "[Lost Jelly] …Find Diver Mario; Abyss Maw still sleeps.",
      "【探索】三枚潮印按序亮起。深渊在低语。",
      "[Explore] Three tide marks lit. The abyss whispers." },
    { MapID::kPolar,
      {{ -1800.f, -900.f }, { -1550.f, -650.f }, { -2000.f, -600.f }},
      -2500.f, -1700.f, -1120.f, -720.f, MobID::kBlizzardWisp,
      "迷路的小企鹅", "Lost Penguin",
      "[迷路的小企鹅] ……去找霜铠残魂。冬之君戴着冰冠。",
      "[Lost Penguin] …Find the Frost Remnant. Winter Sovereign wears ice.",
      "【探索】三枚霜印按序亮起。永冻之心在远方。",
      "[Explore] Three frost marks lit. Permafrost Heart waits afar." },
    { MapID::kJungle,
      {{ 1400.f, -1600.f }, { 1650.f, -1350.f }, { 1200.f, -1300.f }},
      2000.f, -2200.f, 1080.f, -1180.f, MobID::kJungleMite,
      "迷路的小生物", "Lost Critter",
      "[迷路的小生物] 吱……找森境贤者。远古树人仍醒着一半。",
      "[Lost Critter] …Find Grove Sage. The Treant half-wakes.",
      "【探索】三枚藤印按序亮起。树芯在深处跳动。",
      "[Explore] Three vine marks lit. Heartwood beats deeper in." },
    { MapID::kAntHell,
      {{ -1100.f, 1700.f }, { -850.f, 1950.f }, { -1300.f, 2000.f }},
      -1600.f, 2500.f, -820.f, 1280.f, MobID::kWorkerAnt,
      "掉队的工蚁", "Stray Worker",
      "[掉队的工蚁] 吱！找蚁国向导。蚁后在巢心。",
      "[Stray Worker] Skree! Find Ant Guide. The Queen is in the heart.",
      "【探索】三枚巢印按序亮起。信息素在空气里发颤。",
      "[Explore] Three nest marks lit. Pheromones shiver in the air." },
    { MapID::kVolcano,
      {{ 1900.f, 600.f }, { 2150.f, 850.f }, { 1700.f, 900.f }},
      2700.f, 1200.f, 1280.f, 520.f, MobID::kPhantomOrb,
      "迷路的火精灵", "Lost Ember Sprite",
      "[迷路的火精灵] 谢了……找熔火囚人。熔火霸主还在地脉里。",
      "[Lost Ember Sprite] Thanks… find Ash Prisoner. The Overlord is in the leyline.",
      "【探索】三枚灼印按序亮起。硫火在裂隙里跳动。",
      "[Explore] Three sear marks lit. Sulfur fire pulses in the cracks." },
    { MapID::kRoyalCapital,
      {{ -1600.f, -500.f }, { -1350.f, -250.f }, { -1800.f, -200.f }},
      -2300.f, -1100.f, -1080.f, -380.f, MobID::kFallenKnight,
      "迷路的骑士", "Lost Knight",
      "[迷路的骑士] 听好了……找锈铠掮客。陨落之王还坐在空王座上。",
      "[Lost Knight] Listen… find Rust Broker. The Fallen King still claims the throne.",
      "【探索】三枚王印按序亮起。圣印在废墟里发光。",
      "[Explore] Three royal marks lit. The seal glows in the ruins." },
};

constexpr uint8_t kDefCount = (uint8_t) (sizeof(kDefs) / sizeof(kDefs[0]));

struct MapRuntime {
    EntityID pads[3] = { NULL_ENTITY, NULL_ENTITY, NULL_ENTITY };
    float px[3] = {}, py[3] = {};
    float idle_x = 0, idle_y = 0;
    float dest_x = 0, dest_y = 0;
    EntityID spirit = NULL_ENTITY; // map-owned guide — not per-player
    uint32_t spirit_cd = 0;
    uint8_t spawned = 0;
    uint8_t def_index = 0;
};

static MapRuntime g_rt[MapID::kNumMaps];
static uint32_t g_tick = 0;

static MapExploreDef const *def_for(uint8_t map_id) {
    for (uint8_t i = 0; i < kDefCount; ++i)
        if (kDefs[i].map_id == map_id) return &kDefs[i];
    return nullptr;
}

static void place_xy_once(uint8_t map_id, float &x, float &y) {
    Map::resolve_circle_terrain(map_id, x, y, 40.f);
    x = fclamp(x, 200.f, (float) ARENA_WIDTH - 200.f);
    y = fclamp(y, 200.f, (float) ARENA_HEIGHT - 200.f);
}

static void clamp_arena(float &x, float &y) {
    x = fclamp(x, 200.f, (float) ARENA_WIDTH - 200.f);
    y = fclamp(y, 200.f, (float) ARENA_HEIGHT - 200.f);
}

static Entity &spawn_pad(Simulation *sim, float x, float y) {
    return alloc_mob(sim, MobID::kYahahaPad, x, y, NULL_ENTITY, [&](Entity &ent) {
        BitMath::set(ent.flags, EntityFlags::kNoDrops);
        BitMath::set(ent.flags, EntityFlags::kHasCulling);
        BitMath::set(ent.flags, EntityFlags::kNoFriendlyCollision);
        ent.score_reward = 0;
        ent.damage = 0;
        ent.mass = 10000.f;
        ent.health = ent.max_health = 500000.f;
        ent.friction = 1.f;
    });
}

// Guide spirit: invulnerable, heavy, no culling — walks alone to dest.
static Entity &spawn_spirit(Simulation *sim, MobID::T id, float x, float y,
                            char const *name_zh) {
    return alloc_mob(sim, id, x, y, NULL_ENTITY, [&](Entity &ent) {
        BitMath::set(ent.flags, EntityFlags::kNoDrops);
        BitMath::set(ent.flags, EntityFlags::kNoFriendlyCollision);
        // Intentionally NO kHasCulling — unseenDespawn was deleting mid-path.
        BitMath::unset(ent.flags, EntityFlags::kHasCulling);
        BitMath::unset(ent.flags, EntityFlags::kSpawnedFromZone);
        BitMath::unset(ent.flags, EntityFlags::kIsCulled);
        BitMath::unset(ent.flags, EntityFlags::kIsDespawning);
        ent.score_reward = 0;
        ent.damage = 0;
        // Heavy like pads — petals / mobs must not shove or melt it.
        ent.mass = 10000.f;
        ent.friction = 1.f;
        ent.health = ent.max_health = 1e9f;
        ent.set_health_ratio(1);
        ent.immunity_ticks = 60000; // uint16 max-ish; refreshed each tick
        ent.set_nametag_visible(1);
        if (name_zh != nullptr && name_zh[0] != '\0')
            ent.set_name(name_zh);
        ent.set_parent(NULL_ENTITY);
        ent.zone = 0xFF; // treat as special — never zone-despawn
        ent.target = NULL_ENTITY;
    });
}

static void ensure_path_length(uint8_t mid, MapRuntime &rt) {
    float dx = rt.dest_x - rt.idle_x, dy = rt.dest_y - rt.idle_y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len >= kMinPathLen && std::isfinite(len)) return;
    // Terrain snap collapsed path — push dest along original offset direction.
    float ox = dx, oy = dy;
    float ol = std::sqrt(ox * ox + oy * oy);
    if (!(ol > 1.f)) { ox = 1.f; oy = 0.f; ol = 1.f; }
    rt.dest_x = rt.idle_x + ox / ol * kMinPathLen;
    rt.dest_y = rt.idle_y + oy / ol * kMinPathLen;
    place_xy_once(mid, rt.dest_x, rt.dest_y);
    dx = rt.dest_x - rt.idle_x;
    dy = rt.dest_y - rt.idle_y;
    len = std::sqrt(dx * dx + dy * dy);
    if (len < kMinPathLen * 0.5f) {
        // Last resort: offset from idle toward map rim.
        rt.dest_x = fclamp(rt.idle_x + kMinPathLen, 200.f, (float) ARENA_WIDTH - 200.f);
        rt.dest_y = fclamp(rt.idle_y - kMinPathLen * 0.4f, 200.f, (float) ARENA_HEIGHT - 200.f);
        place_xy_once(mid, rt.dest_x, rt.dest_y);
    }
}

static void despawn_spirit(Simulation *sim, MapRuntime &rt) {
    if (sim->ent_alive(rt.spirit))
        sim->request_delete(rt.spirit);
    rt.spirit = NULL_ENTITY;
}

static void award_followers_near(GameInstance *game, MapExploreDef const &def,
                                 uint8_t mid, float x, float y) {
    Simulation *sim = &game->simulation;
    float const r2 = kRewardNearR * kRewardNearR;
    for (Client *client : game->clients) {
        if (client == nullptr || !client->alive()) continue;
        if (!sim->ent_exists(client->camera)) continue;
        Entity &cam = sim->get_ent(client->camera);
        if (!sim->ent_alive(cam.get_player())) continue;
        Entity &player = sim->get_ent(cam.get_player());
        float dx = player.get_x() - x, dy = player.get_y() - y;
        if (dx * dx + dy * dy > r2) continue;

        uint32_t coins = 180;
        client_award_coins(client, coins);
        client_send_loot_toast(client, PetalID::kCoin, (uint16_t) coins, def.spirit_zh);
        uint8_t mats = 0;
        std::string mat_line;
        for (uint8_t i = 0; i < 2; ++i) {
            PetalID::T mat = TreasureSites::pick_explore_mat(mid);
            if (mat == PetalID::kNone) continue;
            if (!client_award_petal(client, mat)) continue;
            ++mats;
            client_send_loot_toast(client, mat, 1, def.spirit_zh);
            char const *pname = (mat < PetalID::kNumPetals && PETAL_DATA[mat].name)
                ? PETAL_DATA[mat].name : "残片";
            if (!mat_line.empty()) mat_line += "、";
            mat_line += pname;
        }
        Chat::send_to(client, ChatChannel::kChatSystem, "", def.arrive_zh);
        Chat::send_to(client, ChatChannel::kChatSystem, "", def.arrive_en);
        Chat::send_to(client, ChatChannel::kChatSystem, "",
            std::string{"【指引】钱币 +"} + std::to_string(coins)
            + (mats ? "，" + mat_line : ""));
    }
}

} // namespace

void WorldExplore::clear_client(Client *client) {
    // Spirits are map-owned — never delete them on player leave/death.
    if (client == nullptr) return;
    client->explore_follower = NULL_ENTITY;
    client->explore_follow_cd = 0;
}

void WorldExplore::on_player_spawned(Client *client) {
    if (client == nullptr || client->game == nullptr) return;
    uint8_t mid = client->game->map_id;
    if (mid >= MapID::kNumMaps || def_for(mid) == nullptr) return;
    if (!client->explore_puzzle_done[mid])
        client->explore_puzzle_step = 0;
    client->explore_follower = NULL_ENTITY;
}

void WorldExplore::spawn_for_map(GameInstance *game) {
    if (game == nullptr) return;
    uint8_t mid = game->map_id;
    if (mid >= MapID::kNumMaps) return;
    MapExploreDef const *def = def_for(mid);
    if (def == nullptr) return;
    MapRuntime &rt = g_rt[mid];
    if (rt.spawned) return;
    Simulation *sim = &game->simulation;

    for (uint8_t i = 0; i < 3; ++i) {
        float x = MAP_SPAWN_X + def->touch_off[i][0];
        float y = MAP_SPAWN_Y + def->touch_off[i][1];
        place_xy_once(mid, x, y);
        rt.px[i] = x;
        rt.py[i] = y;
        rt.pads[i] = spawn_pad(sim, x, y).id;
    }
    rt.idle_x = MAP_SPAWN_X + def->idle_ox;
    rt.idle_y = MAP_SPAWN_Y + def->idle_oy;
    place_xy_once(mid, rt.idle_x, rt.idle_y);
    rt.dest_x = MAP_SPAWN_X + def->dest_ox;
    rt.dest_y = MAP_SPAWN_Y + def->dest_oy;
    place_xy_once(mid, rt.dest_x, rt.dest_y);
    ensure_path_length(mid, rt);
    rt.def_index = (uint8_t) (def - kDefs);
    rt.spirit = NULL_ENTITY;
    rt.spirit_cd = kMapGraceTicks;
    rt.spawned = 1;
}

void WorldExplore::tick(GameInstance *game) {
    if (game == nullptr) return;
    uint8_t mid = game->map_id;
    if (mid >= MapID::kNumMaps) return;
    MapRuntime &rt = g_rt[mid];
    if (!rt.spawned) return;
    MapExploreDef const &def = kDefs[rt.def_index];
    Simulation *sim = &game->simulation;
    ++g_tick;

    if (rt.spirit_cd > 0) --rt.spirit_cd;

    // ---- Map spirit: center → dest, player cannot affect path ----
    if (!rt.spirit.null() && !sim->ent_alive(rt.spirit)) {
        // Unexpected delete (should be rare after invuln) — do NOT instant-respawn.
        rt.spirit = NULL_ENTITY;
        if (rt.spirit_cd < kDeathCooldownTicks)
            rt.spirit_cd = kDeathCooldownTicks;
    } else if (sim->ent_alive(rt.spirit)) {
        Entity &npc = sim->get_ent(rt.spirit);
        // Keep immune / unculled / unparented every tick.
        npc.immunity_ticks = 60000;
        npc.health = npc.max_health;
        npc.damage = 0;
        npc.score_reward = 0;
        npc.velocity = Vector(0, 0);
        npc.acceleration = Vector(0, 0);
        if (!npc.get_parent().null())
            npc.set_parent(NULL_ENTITY);
        BitMath::unset(npc.flags, EntityFlags::kIsCulled);
        BitMath::unset(npc.flags, EntityFlags::kHasCulling);
        BitMath::unset(npc.flags, EntityFlags::kSpawnedFromZone);
        BitMath::unset(npc.flags, EntityFlags::kIsDespawning);

        float nx = npc.get_x(), ny = npc.get_y();
        if (!std::isfinite(nx) || !std::isfinite(ny)) {
            despawn_spirit(sim, rt);
            rt.spirit_cd = kSpiritCooldownTicks;
        } else {
            float dx = rt.dest_x - nx, dy = rt.dest_y - ny;
            float dist2 = dx * dx + dy * dy;
            if (dist2 <= kArriveR * kArriveR) {
                float ax = npc.get_x(), ay = npc.get_y();
                despawn_spirit(sim, rt);
                rt.spirit_cd = kSpiritCooldownTicks;
                award_followers_near(game, def, mid, ax, ay);
            } else {
                float dist = std::sqrt(dist2);
                if (!(dist > 1e-3f) || !std::isfinite(dist)) {
                    despawn_spirit(sim, rt);
                    rt.spirit_cd = kSpiritCooldownTicks;
                } else {
                    float step = kSpiritSpeed < dist ? kSpiritSpeed : dist;
                    float mx = nx + dx / dist * step;
                    float my = ny + dy / dist * step;
                    if (!std::isfinite(mx) || !std::isfinite(my)) {
                        despawn_spirit(sim, rt);
                        rt.spirit_cd = kSpiritCooldownTicks;
                    } else {
                        clamp_arena(mx, my);
                        // Soft terrain — avoid hard snap that yanks spirit back.
                        if ((g_tick & 15u) == 0)
                            Map::resolve_circle_terrain(mid, mx, my, 18.f);
                        npc.set_x(mx);
                        npc.set_y(my);
                    }
                }
            }
        }
    }

    // Probabilistic respawn at zone center (map-level, ignores players).
    if (rt.spirit.null() && rt.spirit_cd == 0) {
        uint8_t any_player = 0;
        for (Client *c : game->clients) {
            if (c != nullptr && c->alive()) { any_player = 1; break; }
        }
        if (any_player && !near_boss(mid, rt.idle_x, rt.idle_y)
            && (g_tick % kSpawnCheckTicks) == 0) {
            if (frand() > kSpawnChance) {
                rt.spirit_cd = kFailCooldownTicks;
            } else {
                float sx = rt.idle_x + (frand() - 0.5f) * 60.f;
                float sy = rt.idle_y + (frand() - 0.5f) * 60.f;
                place_xy_once(mid, sx, sy);
                Entity &npc = spawn_spirit(sim, def.spirit_mob, sx, sy, def.spirit_zh);
                rt.spirit = npc.id;
                // Tip only players near spawn so they can choose to follow.
                float tip_r2 = 900.f * 900.f;
                for (Client *client : game->clients) {
                    if (client == nullptr || !client->alive()) continue;
                    if (!sim->ent_exists(client->camera)) continue;
                    Entity &cam = sim->get_ent(client->camera);
                    if (!sim->ent_alive(cam.get_player())) continue;
                    Entity &player = sim->get_ent(cam.get_player());
                    float tdx = player.get_x() - sx, tdy = player.get_y() - sy;
                    if (tdx * tdx + tdy * tdy > tip_r2) continue;
                    Chat::send_to(client, ChatChannel::kChatSystem, "",
                        std::string{"["} + def.spirit_zh + "] 发光的地方在那边……跟着我。");
                    Chat::send_to(client, ChatChannel::kChatSystem, "",
                        std::string{"["} + def.spirit_en + "] The lights are that way… follow me.");
                }
            }
        }
    }

    // ---- Per-player 3-touch pads (unchanged) ----
    for (Client *client : game->clients) {
        if (client == nullptr || !client->alive()) continue;
        if (!sim->ent_exists(client->camera)) continue;
        Entity &cam = sim->get_ent(client->camera);
        if (!sim->ent_alive(cam.get_player())) continue;
        Entity &player = sim->get_ent(cam.get_player());
        float px = player.get_x(), py = player.get_y();

        if (!client->explore_puzzle_done[mid] && client->explore_puzzle_step < 3) {
            uint8_t step = client->explore_puzzle_step;
            float tdx = px - rt.px[step];
            float tdy = py - rt.py[step];
            if (tdx * tdx + tdy * tdy <= kTouchR * kTouchR) {
                ++client->explore_puzzle_step;
                if (client->explore_puzzle_step >= 3) {
                    client->explore_puzzle_done[mid] = 1;
                    uint32_t coins = 220;
                    client_award_coins(client, coins);
                    client_send_loot_toast(client, PetalID::kCoin, (uint16_t) coins, "探索印记");
                    uint8_t mats = 0;
                    std::string mat_line;
                    for (uint8_t i = 0; i < 3; ++i) {
                        PetalID::T mat = TreasureSites::pick_explore_mat(mid);
                        if (mat == PetalID::kNone) continue;
                        if (!client_award_petal(client, mat)) continue;
                        ++mats;
                        client_send_loot_toast(client, mat, 1, "探索印记");
                        char const *pname = (mat < PetalID::kNumPetals && PETAL_DATA[mat].name)
                            ? PETAL_DATA[mat].name : "残片";
                        if (!mat_line.empty()) mat_line += "、";
                        mat_line += pname;
                    }
                    Chat::send_to(client, ChatChannel::kChatSystem, "", def.touch_done_zh);
                    Chat::send_to(client, ChatChannel::kChatSystem, "", def.touch_done_en);
                    if (mats)
                        Chat::send_to(client, ChatChannel::kChatSystem, "",
                            std::string{"【探索】钱币 +"} + std::to_string(coins)
                            + "，" + mat_line);
                } else {
                    Chat::send_to(client, ChatChannel::kChatSystem, "",
                        std::string{"【探索】印记 "}
                        + std::to_string(client->explore_puzzle_step) + "/3 亮起。");
                }
            }
        }
    }
}
