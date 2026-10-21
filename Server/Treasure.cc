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
constexpr uint32_t kHintCooldownTicks = 12 * TPS;

struct SiteLayout {
    float ox, oy; // offset from map center
    float pad[2][2];
    float crate[2][2];
    char const *name_zh;
    char const *name_en;
    char const *hint_zh;
    char const *hint_en;
    PetalID::T reward;
    uint16_t reward_n;
    uint32_t coins;
};

// One Yahaha site per map — push both crates onto glowing pads.
static SiteLayout const kSites[MapID::kNumMaps] = {
    // Garden — Zelda Korok leaf sage (offsets from hub; mid-ring)
    { 2400.f, -1400.f,
      {{0.f, 0.f}, {90.f, 0.f}}, {{-140.f, 80.f}, {-40.f, 140.f}},
      "叶语科洛克", "Leaf Korok",
      "咦嘿嘿…把两块木箱推到发光记号上，我就给你树叶礼物。",
      "Yahaha! Push both crates onto the glowing marks for a leafy gift.",
      PetalID::kMatLeaf, 4, 90 },
    // Desert — Gerudo/GTA sand merchant
    { -2200.f, 1800.f,
      {{0.f, 0.f}, {0.f, 95.f}}, {{120.f, -60.f}, {180.f, 40.f}},
      "沙丘贩子", "Dune Dealer",
      "嘿，旅行者——把石箱推进沙印里，金币不会亏待你。",
      "Hey traveler — shove those stone crates into the sand seals. Coins await.",
      PetalID::kMatSand, 5, 110 },
    // Ocean — dive merchant / Mario water vibe
    { 1600.f, 2300.f,
      {{0.f, 0.f}, {85.f, 40.f}}, {{-100.f, -90.f}, {40.f, -130.f}},
      "潜水员马尔", "Diver Mario",
      "呜呼！把漂流箱推进光圈，珍珠就归你啦！",
      "Wahoo! Push the drift crates into the light rings for pearls!",
      PetalID::kMatSeashell, 4, 100 },
    // Polar — Dark Souls frost knight
    { -2500.f, -1700.f,
      {{0.f, 0.f}, {100.f, -20.f}}, {{-80.f, 110.f}, {50.f, 150.f}},
      "霜铠残魂", "Frost Remnant",
      "……将石箱置于符文之上。冰晶，便是对勇气的施舍。",
      "...Place the crates upon the runes. Ice crystals for the brave.",
      PetalID::kMatIceCrystal, 4, 120 },
    // Jungle — Elden Ring forest sage
    { 2000.f, -2200.f,
      {{0.f, 0.f}, {-70.f, 80.f}}, {{130.f, 20.f}, {90.f, -100.f}},
      "森境贤者", "Grove Sage",
      "解开藤匣之阵。藤蔓与树脂，乃赐福之物。",
      "Solve the vine-crate rite. Vine and resin shall be your boon.",
      PetalID::kMatVine, 4, 105 },
    // Ant Hell — ant guide
    { -1600.f, 2500.f,
      {{0.f, 0.f}, {80.f, 60.f}}, {{-120.f, -50.f}, {-30.f, -140.f}},
      "蚁国向导", "Ant Guide",
      "吱吱！把甲壳箱推到巢印上，蚁壳给你！",
      "Skree! Push chitin crates onto the nest marks for shells!",
      PetalID::kMatAntShell, 5, 95 },
    // Volcano — Elden Ring ash prisoner
    { 2700.f, 1200.f,
      {{0.f, 0.f}, {-95.f, 30.f}}, {{70.f, -120.f}, {150.f, -40.f}},
      "熔火囚人", "Ash Prisoner",
      "把黑石箱推进灼热印记……我便分你一撮硫火。",
      "Push the blackstone crates into the searing seals... sulfur for you.",
      PetalID::kMatSulfur, 5, 130 },
    // Royal Capital — DS knight + GTA street hustler
    { -2300.f, -1100.f,
      {{0.f, 0.f}, {75.f, -75.f}}, {{-110.f, 90.f}, {40.f, 130.f}},
      "锈铠掮客", "Rust Broker",
      "听好了——把军用箱推进王印，钢和齿轮少不了你的。",
      "Listen — shove the supply crates onto the royal seals. Steel & gears.",
      PetalID::kMatSteel, 4, 140 },
};

struct SiteRuntime {
    EntityID npc = NULL_ENTITY;
    EntityID pads[2] = { NULL_ENTITY, NULL_ENTITY };
    EntityID crates[2] = { NULL_ENTITY, NULL_ENTITY };
    EntityID chests[2] = { NULL_ENTITY, NULL_ENTITY };
    float cx = 0, cy = 0;
    uint8_t spawned = 0;
};

static SiteRuntime g_sites[MapID::kNumMaps];

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
            // Pads are markers only — crates/players pass through
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
    return pad_occupied(sim, site.pads[0], site.crates)
        && pad_occupied(sim, site.pads[1], site.crates);
}

} // namespace

void Treasure::spawn_for_map(GameInstance *game) {
    if (game == nullptr) return;
    uint8_t mid = game->map_id;
    if (mid >= MapID::kNumMaps) return;
    SiteRuntime &rt = g_sites[mid];
    if (rt.spawned) return;
    SiteLayout const &lay = kSites[mid];
    Simulation *sim = &game->simulation;
    float cx = MAP_SPAWN_X + lay.ox;
    float cy = MAP_SPAWN_Y + lay.oy;
    place_xy(mid, cx, cy);
    rt.cx = cx;
    rt.cy = cy;

    float nx = cx - 40.f, ny = cy - 120.f;
    place_xy(mid, nx, ny);
    // Prefer Chinese name in-world; EN players still get EN chat hints.
    Entity &npc = spawn_prop(sim, MobID::kTreasureNpc, nx, ny, lay.name_zh);
    rt.npc = npc.id;

    for (uint8_t i = 0; i < 2; ++i) {
        float px = cx + lay.pad[i][0], py = cy + lay.pad[i][1];
        place_xy(mid, px, py);
        rt.pads[i] = spawn_prop(sim, MobID::kYahahaPad, px, py).id;
        float bx = cx + lay.crate[i][0], by = cy + lay.crate[i][1];
        place_xy(mid, bx, by);
        rt.crates[i] = spawn_prop(sim, MobID::kPushCrate, bx, by).id;
    }
    // Two walk-up chests scattered near the site (Mario/Zelda roadside treasure)
    static float const kChestOff[2][2] = { { 220.f, -160.f }, { -250.f, 200.f } };
    for (uint8_t i = 0; i < 2; ++i) {
        float hx = cx + kChestOff[i][0], hy = cy + kChestOff[i][1];
        place_xy(mid, hx, hy);
        rt.chests[i] = spawn_prop(sim, MobID::kLootChest, hx, hy, "宝箱").id;
    }
    rt.spawned = 1;
}

void Treasure::tick(GameInstance *game) {
    if (game == nullptr) return;
    uint8_t mid = game->map_id;
    if (mid >= MapID::kNumMaps) return;
    SiteRuntime &rt = g_sites[mid];
    if (!rt.spawned) return;
    Simulation *sim = &game->simulation;
    SiteLayout const &lay = kSites[mid];

    // Soft snap: each pad pulls at most one nearest crate (1:1 Yahaha seat)
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

    uint8_t done = puzzle_done(sim, rt);

    for (Client *client : game->clients) {
        if (client == nullptr || !client->alive()) continue;
        if (!sim->ent_exists(client->camera)) continue;
        Entity &cam = sim->get_ent(client->camera);
        if (!sim->ent_alive(cam.get_player())) continue;
        Entity &player = sim->get_ent(cam.get_player());
        float dx = player.get_x() - rt.cx;
        float dy = player.get_y() - rt.cy;
        float dist2 = dx * dx + dy * dy;

        if (client->treasure_hint_cd > 0) --client->treasure_hint_cd;

        // NPC proximity → chat hint
        if (sim->ent_alive(rt.npc) && client->treasure_hint_cd == 0) {
            Entity &npc = sim->get_ent(rt.npc);
            float ndx = player.get_x() - npc.get_x();
            float ndy = player.get_y() - npc.get_y();
            if (ndx * ndx + ndy * ndy <= kNpcHintR * kNpcHintR) {
                std::string tip = std::string{"["} + lay.name_zh + "] "
                    + lay.hint_zh;
                Chat::send_to(client, ChatChannel::kChatSystem, "", tip);
                Chat::send_to(client, ChatChannel::kChatSystem, "",
                    std::string{"["} + lay.name_en + "] " + lay.hint_en);
                client->treasure_hint_cd = kHintCooldownTicks;
            }
        }

        // Walk-up chests (small coin snack; once each)
        for (uint8_t ci = 0; ci < 2; ++ci) {
            if (!sim->ent_alive(rt.chests[ci])) continue;
            uint16_t bit = (uint16_t) (1u << (mid * 2 + ci));
            if (client->treasure_chest_claimed & bit) continue;
            Entity &chest = sim->get_ent(rt.chests[ci]);
            float cdx = player.get_x() - chest.get_x();
            float cdy = player.get_y() - chest.get_y();
            if (cdx * cdx + cdy * cdy > kChestOpenR * kChestOpenR) continue;
            client->treasure_chest_claimed |= bit;
            uint32_t coins = 25 + mid * 8 + ci * 10;
            client_award_coins(client, coins);
            client_send_loot_toast(client, PetalID::kCoin, (uint16_t) coins,
                std::string{"宝箱 · "} + lay.name_zh);
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                std::string{"【宝箱】打开路边宝箱：钱币 +"} + std::to_string(coins));
            Chat::send_to(client, ChatChannel::kChatSystem, "",
                std::string{"[Chest] Opened a roadside chest: +"}
                + std::to_string(coins) + " coins");
        }

        if (!done) continue;
        if (dist2 > kSiteClaimR * kSiteClaimR) continue;
        if (client->treasure_claimed & (1u << mid)) continue;

        client->treasure_claimed |= (uint8_t) (1u << mid);
        uint16_t given = 0;
        for (uint16_t i = 0; i < lay.reward_n; ++i) {
            if (client_award_petal(client, lay.reward)) ++given;
        }
        if (lay.coins > 0) client_award_coins(client, lay.coins);

        std::string title = std::string{"宝藏 · "} + lay.name_zh;
        client_send_loot_toast(client, lay.reward, given, title);

        char const *rname = (lay.reward < PetalID::kNumPetals && PETAL_DATA[lay.reward].name)
            ? PETAL_DATA[lay.reward].name : "?";
        Chat::send_to(client, ChatChannel::kChatSystem, "",
            std::string{"【宝藏】完成推箱：获得 "}
            + rname + " ×" + std::to_string(given)
            + "，钱币 +" + std::to_string(lay.coins));
        Chat::send_to(client, ChatChannel::kChatSystem, "",
            std::string{"[Treasure] Puzzle cleared: "}
            + rname + " x" + std::to_string(given)
            + ", coins +" + std::to_string(lay.coins));
    }
}
