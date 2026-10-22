#pragma once

#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <Helpers/Math.hh>

#include <cstdint>

// Shared Yahaha / roadside treasure layouts (server spawns; client approx for guide sparks).
namespace TreasureSites {
    constexpr uint8_t kMaxSitesPerMap = 8;
    constexpr uint8_t kMaxChestsPerMap = 8;
    constexpr uint8_t kMaxRitualsPerMap = 2;

    enum Kind : uint8_t {
        kRitual = 0,
        kGuide = 1,
        kChestOnly = 2,
    };

    struct SiteDef {
        Kind kind;
        float ox, oy;
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

    struct MapPack {
        SiteDef const *sites;
        uint8_t site_count;
        PetalID::T common_mat;
    };

    inline void hub_xy(float ox, float oy, float &x, float &y) {
        x = MAP_SPAWN_X + ox;
        y = MAP_SPAWN_Y + oy;
    }

    // ---- Garden ----
    inline SiteDef const kGarden[] = {
        { kRitual, 2400.f, -1400.f,
          {{0.f, 0.f}, {90.f, 0.f}}, {{-140.f, 80.f}, {-40.f, 140.f}},
          "叶语科洛克", "Leaf Korok",
          "咦嘿嘿…把两块木箱推到发光记号上。花之心甲虫沉睡时，带叶片去组装叶刃。",
          "Yahaha! Push both crates onto the marks. When the Flower Guardian sleeps, forge Leaf Blade from Leaf Scrap.",
          PetalID::kMatLeaf, 10, 360 },
        { kRitual, -4200.f, 3100.f,
          {{0.f, 0.f}, {0.f, 90.f}}, {{130.f, -50.f}, {50.f, 140.f}},
          "叶语科洛克", "Leaf Korok",
          "外环也有叶礼……树枝也能换礼物。深处有花之心甲虫。",
          "Another leaf rite on the outer ring… twigs welcome. Flower Guardian sleeps deeper in.",
          PetalID::kMatTwig, 10, 400 },
        { kGuide, 900.f, 1800.f, {{}, {}}, {{}, {}},
          "路旁叶语", "Wayside Korok",
          "往东北找发光印记推箱；或按 V 组装叶刃（叶片×5+树枝×2）。",
          "NE for glowing pads; or V-assemble Leaf Blade (Leaf×5 + Twig×2).",
          PetalID::kNone, 0, 0 },
        { kGuide, -1800.f, -900.f, {{}, {}}, {{}, {}},
          "路旁叶语", "Wayside Korok",
          "花园沉睡的君王是花之心甲虫——古心与蜂王蜜在它身上。",
          "Garden's sleeping sovereign is the Flower Guardian — Ancient Heart & Royal Jelly.",
          PetalID::kNone, 0, 0 },
        { kChestOnly, 3500.f, 800.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, -900.f, 3200.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 1100.f, -3600.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
    };

    // ---- Desert ----
    inline SiteDef const kDesert[] = {
        { kRitual, -2200.f, 1800.f,
          {{0.f, 0.f}, {0.f, 95.f}}, {{120.f, -60.f}, {180.f, 40.f}},
          "沙丘贩子", "Dune Dealer",
          "嘿——石箱推进沙印。沙虫君主沉睡时，带沙粒去组装沙刺。",
          "Shove crates into sand seals. When the Sandworm Monarch sleeps, forge Sand Spike from Sand Grain.",
          PetalID::kMatSand, 12, 440 },
        { kRitual, 4800.f, -2800.f,
          {{0.f, 0.f}, {85.f, 40.f}}, {{-120.f, 70.f}, {40.f, -140.f}},
          "沙丘贩子", "Dune Dealer",
          "外环沙印也收毒刺。深处有沙虫君主。",
          "Outer seals take venom barbs too. Sandworm Monarch waits deeper.",
          PetalID::kMatVenomBarb, 8, 480 },
        { kGuide, 1500.f, -1200.f, {{}, {}}, {{}, {}},
          "沙路掮客", "Sand Road Broker",
          "西南找沙印推箱；V 组装沙刺（沙粒×6+毒刺×2）。",
          "SW for sand seals; V-assemble Sand Spike (Sand×6 + Venom Barb×2).",
          PetalID::kNone, 0, 0 },
        { kGuide, -3400.f, -2000.f, {{}, {}}, {{}, {}},
          "沙路掮客", "Sand Road Broker",
          "沙漠君王是沙虫君主——沙虫核心与沙暴之眼。",
          "Desert sovereign: Sandworm Monarch — Sandworm Core & Sandstorm Eye.",
          PetalID::kNone, 0, 0 },
        { kChestOnly, 2800.f, 2600.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, -500.f, -3800.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 4200.f, 900.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
    };

    // ---- Ocean ----
    inline SiteDef const kOcean[] = {
        { kRitual, 1600.f, 2300.f,
          {{0.f, 0.f}, {85.f, 40.f}}, {{-100.f, -90.f}, {40.f, -130.f}},
          "潜水员马尔", "Diver Mario",
          "呜呼！漂流箱推进光圈。深渊巨口沉睡时，带水珠去组装水泡弹。",
          "Wahoo! Push drift crates into rings. When Abyss Maw sleeps, forge Water Bubble.",
          PetalID::kMatSeashell, 10, 400 },
        { kRitual, -3900.f, -2600.f,
          {{0.f, 0.f}, {-80.f, 50.f}}, {{110.f, 60.f}, {-40.f, 150.f}},
          "潜水员马尔", "Diver Mario",
          "外环光圈收海盐。深处有深渊巨口。",
          "Outer rings take sea salt. Abyss Maw waits in the deep.",
          PetalID::kMatSeaSalt, 12, 440 },
        { kGuide, -800.f, 1700.f, {{}, {}}, {{}, {}},
          "礁边向导", "Reef Guide",
          "东南找光圈；V 组装水泡弹（水珠×6+海盐×2）。",
          "SE for light rings; V-assemble Water Bubble (Drop×6 + Salt×2).",
          PetalID::kNone, 0, 0 },
        { kGuide, 3000.f, -1500.f, {{}, {}}, {{}, {}},
          "礁边向导", "Reef Guide",
          "海洋君王是深渊巨口——潮汐之核与深海珍珠。",
          "Ocean sovereign: Abyss Maw — Tide Core & Abyss Pearl.",
          PetalID::kNone, 0, 0 },
        { kChestOnly, -2500.f, 900.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 4500.f, 1800.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 200.f, -4200.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
    };

    // ---- Polar / Snow ----
    inline SiteDef const kPolar[] = {
        { kRitual, -2500.f, -1700.f,
          {{0.f, 0.f}, {100.f, -20.f}}, {{-80.f, 110.f}, {50.f, 150.f}},
          "霜铠残魂", "Frost Remnant",
          "……石箱置于符文。冬之君沉睡时，带冰晶去组装冰锥。",
          "...Crates upon the runes. When Winter Sovereign sleeps, forge Ice Spike.",
          PetalID::kMatIceCrystal, 10, 480 },
        { kRitual, 4100.f, 2900.f,
          {{0.f, 0.f}, {-90.f, 30.f}}, {{140.f, -40.f}, {-50.f, -130.f}},
          "霜铠残魂", "Frost Remnant",
          "外环符文收雪绒。深处有冬之君。",
          "Outer runes take snow fluff. Winter Sovereign waits deeper.",
          PetalID::kMatSnowFluff, 12, 460 },
        { kGuide, 1200.f, -2200.f, {{}, {}}, {{}, {}},
          "霜路残影", "Frost Road Echo",
          "西北找符文；V 组装冰锥（冰晶×5+寒铁×2）。",
          "NW for runes; V-assemble Ice Spike (Crystal×5 + Cold Iron×2).",
          PetalID::kNone, 0, 0 },
        { kGuide, -1600.f, 2400.f, {{}, {}}, {{}, {}},
          "霜路残影", "Frost Road Echo",
          "雪原君王是冬之君——永冻之心与暴雪之眼。",
          "Snow sovereign: Winter Sovereign — Permafrost Heart & Blizzard Eye.",
          PetalID::kNone, 0, 0 },
        { kChestOnly, 3200.f, -800.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, -3800.f, 600.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 700.f, 3800.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
    };

    // ---- Jungle ----
    inline SiteDef const kJungle[] = {
        { kRitual, 2000.f, -2200.f,
          {{0.f, 0.f}, {-70.f, 80.f}}, {{130.f, 20.f}, {90.f, -100.f}},
          "森境贤者", "Grove Sage",
          "解开藤匣。远古树人沉睡时，带藤蔓去组装荆藤。",
          "Solve the vine-crate rite. When Ancient Treant sleeps, forge Thorn Vine.",
          PetalID::kMatVine, 10, 420 },
        { kRitual, -4500.f, 2400.f,
          {{0.f, 0.f}, {95.f, 0.f}}, {{-100.f, 90.f}, {60.f, 150.f}},
          "森境贤者", "Grove Sage",
          "外环藤匣收树脂。深处有远古树人。",
          "Outer vine-crates take resin. Ancient Treant waits deeper.",
          PetalID::kMatResin, 8, 460 },
        { kGuide, -1000.f, -1600.f, {{}, {}}, {{}, {}},
          "林缘贤影", "Grove Wayside",
          "北向找藤匣；V 组装荆藤（藤蔓×6+树脂×2）。",
          "North for vine rites; V-assemble Thorn Vine (Vine×6 + Resin×2).",
          PetalID::kNone, 0, 0 },
        { kGuide, 2800.f, 2100.f, {{}, {}}, {{}, {}},
          "林缘贤影", "Grove Wayside",
          "丛林君王是远古树人——远古树芯与树脂。",
          "Jungle sovereign: Ancient Treant — Heartwood Chip & Resin.",
          PetalID::kNone, 0, 0 },
        { kChestOnly, -2800.f, -900.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 4000.f, 400.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 600.f, 3600.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
    };

    // ---- Ant Hell ----
    inline SiteDef const kAntHell[] = {
        { kRitual, -1600.f, 2500.f,
          {{0.f, 0.f}, {80.f, 60.f}}, {{-120.f, -50.f}, {-30.f, -140.f}},
          "蚁国向导", "Ant Guide",
          "吱吱！甲壳箱推到巢印。蚁后沉睡时，带蚁壳去组装蚁牙。",
          "Skree! Push crates onto nest marks. When Queen Ant sleeps, forge Ant Fang.",
          PetalID::kMatAntShell, 12, 380 },
        { kRitual, 4300.f, -2200.f,
          {{0.f, 0.f}, {-70.f, 80.f}}, {{100.f, -70.f}, {40.f, 140.f}},
          "蚁国向导", "Ant Guide",
          "外环巢印收触角。深处有蚁后。",
          "Outer nest marks take antenna scrap. Queen Ant waits deeper.",
          PetalID::kMatAntenna, 8, 420 },
        { kGuide, 800.f, -1400.f, {{}, {}}, {{}, {}},
          "穴道哨蚁", "Tunnel Scout",
          "东北找巢印；V 组装蚁牙（蚁壳×4+触角×3）。",
          "NE for nest marks; V-assemble Ant Fang (Shell×4 + Antenna×3).",
          PetalID::kNone, 0, 0 },
        { kGuide, -3000.f, -800.f, {{}, {}}, {{}, {}},
          "穴道哨蚁", "Tunnel Scout",
          "蚁穴君王是蚁后——群巢之心与蚁后信息素。",
          "Ant Hell sovereign: Queen Ant — Hive Heart & Queen Pheromone.",
          PetalID::kNone, 0, 0 },
        { kChestOnly, 2200.f, 1800.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, -4000.f, 1200.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 1000.f, -3900.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
    };

    // ---- Volcano ----
    inline SiteDef const kVolcano[] = {
        { kRitual, 2700.f, 1200.f,
          {{0.f, 0.f}, {-95.f, 30.f}}, {{70.f, -120.f}, {150.f, -40.f}},
          "熔火囚人", "Ash Prisoner",
          "黑石箱推进灼热印记……熔火霸主沉睡时，带硫磺去组装火球。",
          "Push crates into searing seals… When Molten Overlord sleeps, forge Fireball.",
          PetalID::kMatSulfur, 12, 520 },
        { kRitual, -4000.f, -3000.f,
          {{0.f, 0.f}, {80.f, -50.f}}, {{-130.f, 40.f}, {-40.f, 140.f}},
          "熔火囚人", "Ash Prisoner",
          "外环印记收火晶。深处有熔火霸主。",
          "Outer seals take fire crystal. Molten Overlord waits deeper.",
          PetalID::kMatFireCrystal, 8, 500 },
        { kGuide, -900.f, 2000.f, {{}, {}}, {{}, {}},
          "灰道囚人", "Ash Road Prisoner",
          "东向找灼印；V 组装火球（火晶×5+硫磺×3）。",
          "East for searing seals; V-assemble Fireball (Crystal×5 + Sulfur×3).",
          PetalID::kNone, 0, 0 },
        { kGuide, 1800.f, -2800.f, {{}, {}}, {{}, {}},
          "灰道囚人", "Ash Road Prisoner",
          "火山君王是熔火霸主——地脉熔核与凤凰余烬。",
          "Volcano sovereign: Molten Overlord — Leyline Core & Phoenix Ember.",
          PetalID::kNone, 0, 0 },
        { kChestOnly, -2600.f, 800.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 3900.f, -900.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, 500.f, 4000.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
    };

    // ---- Royal Capital ----
    inline SiteDef const kRoyal[] = {
        { kRitual, -2300.f, -1100.f,
          {{0.f, 0.f}, {75.f, -75.f}}, {{-110.f, 90.f}, {40.f, 130.f}},
          "锈铠掮客", "Rust Broker",
          "听好了——军用箱推进王印。陨落之王沉睡时，带精钢去组装精钢剑。",
          "Listen — shove crates onto royal seals. When Fallen King sleeps, forge Steel Sword.",
          PetalID::kMatSteel, 10, 560 },
        { kRitual, 4600.f, 2700.f,
          {{0.f, 0.f}, {-85.f, 40.f}}, {{120.f, -60.f}, {-30.f, 140.f}},
          "锈铠掮客", "Rust Broker",
          "外环王印收齿轮。深处有陨落之王。",
          "Outer seals take gears. Fallen King waits deeper.",
          PetalID::kMatGear, 10, 540 },
        { kGuide, 1000.f, 1600.f, {{}, {}}, {{}, {}},
          "街角掮客", "Street Broker",
          "西向找王印；V 组装精钢剑（精钢×6+齿轮×3）。",
          "West for royal seals; V-assemble Steel Sword (Steel×6 + Gear×3).",
          PetalID::kNone, 0, 0 },
        { kGuide, -1700.f, 2800.f, {{}, {}}, {{}, {}},
          "街角掮客", "Street Broker",
          "王城君王是陨落之王——王权核心与皇家圣印。流亡商在商店卖残片。",
          "Capital sovereign: Fallen King — Sovereign Core & Royal Seal. Exiles sell scrap in the shop.",
          PetalID::kNone, 0, 0 },
        { kChestOnly, 3000.f, -1800.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, -3600.f, 400.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
        { kChestOnly, -600.f, -3600.f, {{}, {}}, {{}, {}},
          "神秘宝箱", "Mystery Chest", "", "", PetalID::kNone, 0, 0 },
    };

    inline PetalID::T common_mat_for(uint8_t mid) {
        static PetalID::T const kMats[MapID::kNumMaps] = {
            PetalID::kMatLeaf, PetalID::kMatSand, PetalID::kMatWaterDrop,
            PetalID::kMatIceCrystal, PetalID::kMatVine, PetalID::kMatAntShell,
            PetalID::kMatSulfur, PetalID::kMatSteel,
        };
        if (mid >= MapID::kNumMaps) mid = 0;
        return kMats[mid];
    }

    // Ritual / craft secondary scrap for each map theme (no boss cores).
    inline PetalID::T secondary_mat_for(uint8_t mid) {
        static PetalID::T const kMats[MapID::kNumMaps] = {
            PetalID::kMatTwig, PetalID::kMatVenomBarb, PetalID::kMatSeaSalt,
            PetalID::kMatSnowFluff, PetalID::kMatResin, PetalID::kMatAntenna,
            PetalID::kMatFireCrystal, PetalID::kMatGear,
        };
        if (mid >= MapID::kNumMaps) mid = 0;
        return kMats[mid];
    }

    // Extra common-tier scrap that still fits the biome (still no boss uniques).
    inline PetalID::T tertiary_mat_for(uint8_t mid) {
        static PetalID::T const kMats[MapID::kNumMaps] = {
            PetalID::kMatPollenDust, PetalID::kMatCactusSap, PetalID::kMatSeashell,
            PetalID::kMatColdIron, PetalID::kMatSpore, PetalID::kMatFormicAcid,
            PetalID::kMatLavaRock, PetalID::kMatHolyShard,
        };
        if (mid >= MapID::kNumMaps) mid = 0;
        return kMats[mid];
    }

    // Explore / world-event drops: always this map's craft pool.
    // ~55% primary, ~30% secondary, ~15% tertiary.
    inline PetalID::T pick_explore_mat(uint8_t mid) {
        float r = frand();
        if (r < 0.55f) return common_mat_for(mid);
        if (r < 0.85f) return secondary_mat_for(mid);
        return tertiary_mat_for(mid);
    }

    inline MapPack pack(uint8_t map_id) {
        static MapPack const kPacks[MapID::kNumMaps] = {
            { kGarden, (uint8_t)(sizeof(kGarden) / sizeof(kGarden[0])), PetalID::kMatLeaf },
            { kDesert, (uint8_t)(sizeof(kDesert) / sizeof(kDesert[0])), PetalID::kMatSand },
            { kOcean, (uint8_t)(sizeof(kOcean) / sizeof(kOcean[0])), PetalID::kMatWaterDrop },
            { kPolar, (uint8_t)(sizeof(kPolar) / sizeof(kPolar[0])), PetalID::kMatIceCrystal },
            { kJungle, (uint8_t)(sizeof(kJungle) / sizeof(kJungle[0])), PetalID::kMatVine },
            { kAntHell, (uint8_t)(sizeof(kAntHell) / sizeof(kAntHell[0])), PetalID::kMatAntShell },
            { kVolcano, (uint8_t)(sizeof(kVolcano) / sizeof(kVolcano[0])), PetalID::kMatSulfur },
            { kRoyal, (uint8_t)(sizeof(kRoyal) / sizeof(kRoyal[0])), PetalID::kMatSteel },
        };
        if (map_id >= MapID::kNumMaps) map_id = 0;
        return kPacks[map_id];
    }
}
