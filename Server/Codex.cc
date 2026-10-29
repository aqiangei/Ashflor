#include <Server/Codex.hh>

#include <Shared/Craft.hh>
#include <Shared/MapGen.hh>
#include <Shared/RarityScale.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>
#include <Shared/MobDescZh.inc>
#include <Shared/MobNameZh.inc>
#include <Shared/PetalDescZh.inc>
#include <Shared/PetalNameZh.inc>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

static void esc(std::string &out, char const *s) {
    if (s == nullptr) { out += ""; return; }
    for (char const *p = s; *p; ++p) {
        unsigned char c = (unsigned char) *p;
        if (c == '"' || c == '\\') { out += '\\'; out += (char) c; }
        else if (c < 0x20) {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "\\u%04x", c);
            out += buf;
        } else out += (char) c;
    }
}

static char const *rarity_zh(uint8_t r) {
    static char const *const k[] = {
        "普通", "罕见", "稀有", "史诗", "传奇", "神话", "究极", "超凡", "唯一"
    };
    return r < RarityID::kNumRarities ? k[r] : "?";
}

static char const *rarity_en(uint8_t r) {
    static char const *const k[] = {
        "Common", "Unusual", "Rare", "Epic", "Legendary", "Mythic", "Ultra", "Super", "Unique"
    };
    return r < RarityID::kNumRarities ? k[r] : "?";
}

static void jnum(std::string &out, float v) {
    char buf[48];
    if (v == (float) (int) v)
        std::snprintf(buf, sizeof(buf), "%d", (int) v);
    else
        std::snprintf(buf, sizeof(buf), "%.4g", (double) v);
    out += buf;
}

static void jkv_f(std::string &out, char const *k, float v, uint8_t &first) {
    if (!first) out += ',';
    first = 0;
    out += '"';
    out += k;
    out += "\":";
    jnum(out, v);
}

static void jkv_u(std::string &out, char const *k, unsigned v, uint8_t &first) {
    if (!first) out += ',';
    first = 0;
    out += '"';
    out += k;
    out += "\":";
    out += std::to_string(v);
}

static void jkv_s(std::string &out, char const *k, char const *v, uint8_t &first) {
    if (!first) out += ',';
    first = 0;
    out += '"';
    out += k;
    out += "\":\"";
    esc(out, v ? v : "");
    out += '"';
}

static char const *equip_name(uint8_t e) {
    switch (e) {
        case EquipmentFlags::kThirdEye: return "third_eye";
        case EquipmentFlags::kAntennae: return "antennae";
        case EquipmentFlags::kObserver: return "observer";
        case EquipmentFlags::kCutter: return "cutter";
        case EquipmentFlags::kAccelerator: return "accelerator";
        case EquipmentFlags::kMagnet: return "magnet";
        default: return "none";
    }
}

static char const *rot_name(uint8_t r) {
    switch (r) {
        case PetalAttributes::kNoRot: return "no_rot";
        case PetalAttributes::kFollowRot: return "follow";
        default: return "passive";
    }
}

struct MapIntro {
    char const *zh;
    char const *en;
    char const *tagline_zh;
    char const *tagline_en;
    char const *body_zh;
    char const *body_en;
    uint32_t color;
};

static MapIntro const kCodexMaps[MapID::kNumMaps] = {
    { "花园", "Garden",
      "叶绿与虫鸣的起点", "Where chlorophyll and bugs begin",
      "八域之门。浅区练手、深区渐险。叶语科洛克与路旁向导驻守；花粉、刺针与基础花瓣在此扎根。按 G 可见稀有度分区色块，与刷怪难度同源。",
      "Gateway of the eight realms. Easy shallows, harder depths. Leaf Korok and wayside guides; pollen, thorns and starter petals take root. Press G for rarity zones that match spawn difficulty.",
      0xff8FC777 },
    { "沙漠", "Desert",
      "热浪与甲壳的旷野", "Heat haze and carapace flats",
      "沙丘、仙人掌与蝎群。沙路掮客指引仪式；沙暴蜈蚣与岩石系威胁横行。外环掉落更肥，但走位稍有不慎就会被包围。",
      "Dunes, cacti and scorpions. Sand-road brokers point to rituals; sandstorm centipedes and rock threats roam. Outer rings drop richer loot but punish poor positioning.",
      0xffd4a84b },
    { "海洋", "Ocean",
      "潮汐与珊瑚的深蓝", "Tides and coral deeps",
      "礁盘、水母与刺豚。礁边向导与护送水母穿行洋流。泡泡、导弹与水生系花瓣在此大放异彩；注意潮区密度。",
      "Reefs, jellyfish and puffers. Reef guides and escort jellies ride currents. Bubbles, missiles and aquatic petals shine; watch tide-zone density.",
      0xff00669c },
    { "雪原", "Polar",
      "霜线与静默的白原", "Frost line and silent white",
      "冻土、企鹅护送与霜路残影。低温区怪群粘滞感强，重型与岩系花瓣更吃香。仪式落点常藏在雪脊背面。",
      "Tundra, penguin escorts and frost-road echoes. Cold packs feel sticky; heavy and rock petals excel. Ritual sites often hide behind snow ridges.",
      0xff9aa3ab },
    { "丛林", "Jungle",
      "藤蔓与密林的呼吸", "Vines and canopy breath",
      "树冠阴影下的甲虫、黄蜂与迷路 critter。林缘贤影提供线索；叶、花粉与毒系在此兑现。迷路护送任务会穿过多段藤廊。",
      "Beetles, wasps and lost critters under canopy shade. Forest-edge sages give clues; leaf, pollen and poison pay off. Escort routes weave through vine halls.",
      0xff40916c },
    { "蚁穴", "Ant Hell",
      "蚁道与女王的地底", "Tunnels and the queen below",
      "工蚁、兵蚁、蚁后与穴道哨蚁。走廊狭窄，蜈蚣与蚁潮考验清场节奏。Boss 女王是本图旗帜战。",
      "Workers, soldiers, queens and tunnel sentries. Tight corridors; centipedes and ant waves test clear tempo. The Queen is this map's banner fight.",
      0xffa98467 },
    { "火山", "Volcano",
      "熔岩与灰烬的裂谷", "Lava rifts and ash",
      "灰道囚人、余烬护送与高威胁熔岩生物。陨石、火系与高伤花瓣对口；外环唯一掉落占比更高，也更致命。",
      "Ash-road captives, ember escorts and high-threat lava fauna. Meteor, fire and high-damage petals fit; outer Unique odds rise with the danger.",
      0xff4a4e69 },
    { "王城", "Royal Capital",
      "城墙、禁苑与王冠", "Walls, gardens and the crown",
      "按稀有度切开的城区：外墙→街区→禁苑→军营→废墟→王宫。街角掮客与骑士护送游走其间；终局压力与终局掉落同在。",
      "Districts by rarity: outer wall → quarter → garden → barracks → ruins → palace. Corner brokers and knight escorts; endgame pressure and endgame loot together.",
      0xffc9a227 },
    { "蚁巢", "Ant Nest",
      "三族分庭的洞穴", "Three-faction nest caverns",
      "玩家手绘巢穴：左火蚁、中黑蚁、右白蚁。窄道连通各室；兵白蚁掉落三角形。勿与旧「蚁穴」混淆。",
      "Hand-drawn nest: fire ants left, black ants center, termites right. Narrow tunnels link chambers; soldier termites drop Triangle. Not the old Ant Hell.",
      0xffc48a55 },
    { "密林", "Wildwood",
      "旧日丛林的深处", "The old jungle, deeper still",
      "从一级丛林西门进入的二级密林。保留原 Ashflor 丛林生态：螳螂、石卫、毒蛙与古树核心。死亡后在密林出生点重生。",
      "L2 wilds via Jungle's west gate. Keeps the old Ashflor jungle roster: mantis, stone guardians, toads and the treant heart. Deaths respawn at the Wildwood hub.",
      0xff2d6a4f },
    { "下水道", "Sewers",
      "污泥与管道", "Sludge and pipes",
      "一级花园南门进入。蟑螂、银鱼、苍蝇、蛾与垃圾堆；便便与翅膀类掉落。死亡后在下水道出生点重生。",
      "Enter via Garden's south gate. Roaches, silverfish, flies, moths and garbage; Poo and wing drops. Deaths respawn at the Sewer hub.",
      0xff6B7068 },
    { "繁花园", "Prime Garden",
      "官方花园正史", "The official garden chronicle",
      "一级花园北门进入的二级官方花园。石头桩与经典蜜蜂/瓢虫/黄蜂谱系。死亡后在繁花园出生点重生。",
      "L2 official garden via Garden's north gate. Rock stakes and classic bee/ladybug/hornet packs. Deaths respawn at the Prime hub.",
      0xff8FC777 },
    { "沙海", "Sand Seas",
      "官方沙漠深处", "The official desert depths",
      "一级沙漠东门进入。仙人掌桩、蝎、沙暴与沙丘甲虫。死亡后在沙海出生点重生。",
      "Enter via Desert's east gate. Cactus stakes, scorpions, sandstorms and dune beetles. Deaths respawn at the Sand Seas hub.",
      0xffd4a84b },
    { "深海", "Deep Ocean",
      "官方海洋深处", "The official ocean depths",
      "一级海洋东门进入。海绵/泡泡桩与水蛭、海鳗、深渊鲨。死亡后在深海出生点重生。",
      "Enter via Ocean's east gate. Sponge/Bubble stakes plus leeches, eels and abyss sharks. Deaths respawn at the Deep hub.",
      0xff00669c },
    { "工厂", "Factory",
      "齿轮与废铁", "Gears and scrap",
      "三级。从繁花园西门进入。Mecha 花/蛛/蜂/蟹、毒桶与组装台；电池/齿轮/激光掉落。死亡后在工厂出生点重生。",
      "L3 via Prime Garden's west gate. Mecha Flower/Spider/Wasp/Crab, Barrels and Assemblers; Battery/Cogwheel/Laser drops. Deaths respawn at the Factory hub.",
      0xff7A8088 },
    { "金字塔", "Pyramid",
      "陵寝与圣甲虫", "Tombs and scarabs",
      "三级。从繁花园东门进入。陵墓、木乃伊甲虫、法老甲虫与圣甲虫；纳扎尔/法老冠掉落。死亡后在金字塔出生点重生。",
      "L3 via Prime Garden's east gate. Tombs, mummy/pharaoh beetles and scarabs; Nazar/Pharaoh Crown drops. Deaths respawn at the Pyramid hub.",
      0xffE2C48A },
    { "地狱", "Hel",
      "暗黑标记的彼岸", "Where Dark Mark sends you",
      "Mark 死亡 Continue 或点尸进入。全员 PvP（无离开封锁）。合成面板变为 Plinko（扑克筹码）。专属 Hel 甲虫/蛛/蜂/蜈蚣、赌徒与幽灵。北门回花园。",
      "Enter via Marked death Continue or corpse Mark. Full FFA PvP, no exit lock. Craft panel becomes Plinko (Poker Chips). Hel insects, Gambler, Ghost. North gate to Garden.",
      0xff3A1A22 },
};

struct MechChapter {
    char const *id;
    char const *title_zh;
    char const *title_en;
    char const *summary_zh;
    char const *summary_en;
    char const *detail_zh;
    char const *detail_en;
};

static MechChapter const MECHS[] = {
    { "rarity", "稀有度梯子", "Rarity ladder",
      "普通→罕见→稀有→史诗→传奇→神话→究极→超凡→唯一",
      "Common→Unusual→Rare→Epic→Legendary→Mythic→Ultra→Super→Unique",
      "同名花瓣/怪物可跨稀有度升阶。合成优先同族最近 ID。怪掉落稀有度默认自身/−1/−2；世界事件与宝箱另走 Ultra/Super/Unique 占比池。",
      "Same-name petals/mobs climb rarities. Craft prefers nearest same-name ID. Mob drop rarity is self/−1/−2; events and chests use Ultra/Super/Unique pools." },
    { "craft", "合成与材料库", "Craft & material bank",
      "五合一升阶；失败有损耗；唯一可继续赌",
      "5-merge upgrades; failures cost; Unique can keep gambling",
      "背包副槽与 craft_bank 互通。合成成功按阶梯写入结果瓣；高级合成（超凡+）计入全服播报与排行榜。金币有独立堆叠上限。成功率：普→罕100%、罕→稀72%、稀→史36%、史→传18%、传→神8%、神→究3%、究→超1.6%、超→唯0.8%。",
      "Secondary slots share craft_bank. Success writes the next ladder petal; Super+ crafts hit broadcasts and leaderboards. Coin has its own stack cap. Success: C→U 100%, U→R 72%, R→E 36%, E→L 18%, L→M 8%, M→Ultra 3%, Ultra→Super 1.6%, Super→Unique 0.8%." },
    { "loadout", "装备与配装", "Loadout & presets",
      "主槽作战 / 副槽背包；K·L 云端配装",
      "Primary combat / secondary bag; K·L cloud presets",
      "等级与天赋决定主槽数。死亡后仍可整理装备与切换配装。空预设进 craft_bank，避免吞瓣。组装界面可悬停查看花瓣信息。",
      "Level and talents set primary slot count. After death you can still rearrange and swap presets. Empty presets go to craft_bank so petals are not eaten. Assembly UI shows petal tips on hover." },
    { "combat", "战斗与瞄准", "Combat & aiming",
      "花瓣环绕、攻击/防御姿态、天赋乘算",
      "Orbiting petals, attack/defend stances, talent multipliers",
      "体魄加生命，冷却天赋与黄金之叶乘算刷新。月亮系独立绕飞并可甩出；磁铁将掉落吸入背包。反转攻击交换攻防方向。",
      "Vitality raises HP; cooldown talent multiplies with Golden Leaf. Moon petals orbit separately and can be flung; Magnet pulls drops into the bag. Invert Attack swaps attack/defend directions." },
    { "xp", "经验与等级", "XP & levels",
      "击杀给分；等级决定血量、槽位与天赋上限",
      "Kills give score; level gates HP, slots and talent caps",
      "经验 = 怪 xp × 区域倍率 × 生成稀有度倍率 × 击杀倍率。分数高水位存档；溢出钳到 uint32 上限。排行榜分数/等级同步展示。",
      "XP = mob xp × zone × spawn-rarity × kill mult. Score high-water is saved; overflow clamps to uint32 max. Leaderboards show score and level." },
    { "talent", "天赋", "Talents",
      "体魄、转速、冷却、瓣甲、射程、磁铁等",
      "Vitality, rotation, cooldown, petal armor, range, magnet…",
      "点数随等级解锁。体魄显著抬血；冷却影响所有花瓣回复；磁铁扩大吸入半径。槽位数由天赋与等级共同决定。",
      "Points unlock with level. Vitality raises HP a lot; cooldown affects every petal reload; magnet widens pickup. Slot count comes from talents and level together." },
    { "drop", "掉落规则", "Drop rules",
      "怪掉看自身表；事件宝箱走全池高稀有度",
      "Mobs use their table; events/chests use high-rarity full pools",
      "怪物：掉落稀有度=自身/−1/−2（20%/60%/20%），材料来自该怪掉落表加权。事件/宝箱/仪式：究极≈55%、超凡≈35%、唯一≈10%（档位微调），约 88% 抽全花瓣池。详见 rules 字段。",
      "Mobs: drop rarity = self/−1/−2 (20%/60%/20%); seeds weighted from that mob's table. Events/chests/rituals: Ultra≈55% / Super≈35% / Unique≈10% (tier nudge), ~88% full petal pool. See rules." },
    { "world", "世界事件与探索", "World events & explore",
      "八域随机事件、护送、路旁贤者、仪式与宝箱",
      "Per-map events, escorts, wayside sages, rituals and chests",
      "事件实体（逃离灌木、列高蘑菇、传说印记等）带 ThemeArt 程序美术。探索 NPC / 护送 / Yahaha 垫板、木箱、宝箱与幻影宝箱均可交互。道具标 zone=0xFF，不被野怪缓清误删。",
      "Event entities (flee bush, height mushrooms, lore marks…) use ThemeArt. Explore NPCs / escorts / Yahaha pads, crates, chests and mirage chests are interactive. Props use zone=0xFF so wild GC will not delete them." },
    { "social", "社交", "Social",
      "附近/队伍/公会/私聊/全服；公会与组队",
      "Near/party/guild/DM/global; guilds and parties",
      "聊天写入 chatlog；击杀与高级合成进全服播报。组队共享击杀经验。排行榜含在线时长、等级、发言、Boss 击杀、高级合成。",
      "Chat goes to chatlog; kills and high crafts hit global broadcasts. Parties share kill XP. Leaderboards cover playtime, level, chat, boss kills, high crafts." },
    { "bonus", "加成时间", "Bonus time",
      "每日与离线加成，最长数小时",
      "Daily and offline bonus, capped at several hours",
      "需手动开启。每日有上限，离线累计另计，总存储有封顶。用于加快成长节奏，不改变掉落表。",
      "Must be started manually. Daily cap plus separate offline accrual; total storage is capped. Speeds growth; does not change drop tables." },
};

struct ArtKind {
    char const *id;
    char const *zh;
    char const *en;
    char const *role_zh;
    char const *role_en;
};

static ArtKind const ARTS[] = {
    { "main_npc", "主线贤者", "Main sage", "每图一位主题 NPC，对话与探索奖励入口", "One themed NPC per map; dialogue and explore rewards" },
    { "wayside", "路旁向导", "Wayside guide", "路边情报与轻量互动", "Roadside tips and light interaction" },
    { "escort", "护送精灵", "Escort spirit", "迷路/掉队单位，护送过图", "Lost units you escort across the map" },
    { "pad", "Yahaha 垫板", "Yahaha pad", "踩踏触发的地图机关", "Step-triggered map gadgets" },
    { "crate", "推箱", "Push crate", "可推动的主题木箱", "Pushable themed crates" },
    { "chest", "宝箱", "Loot chest", "打碎开启的 Yahaha 宝箱", "Kill-to-open Yahaha chests" },
    { "mirage", "幻影箱", "Mirage chest", "半透明诱饵箱", "Translucent decoy chests" },
    { "flee", "逃离事件", "Flee event", "受惊逃窜的灌木/沙丘/海藻等", "Spooked fleeing bush / dune / kelp…" },
    { "height", "列高事件", "Height event", "蘑菇/仙人掌/贝壳等高度序列", "Ordered mushrooms / cactus / shells…" },
    { "lore", "传说印记", "Lore mark", "种子、石碑、足迹等轻叙事点", "Seeds, steles, footprints — light lore" },
};

#include "CodexWorld.inc"

// Structured numbers + code flow for each mechanics chapter (from live constants).
static void emit_mech_payload(std::string &out, char const *id) {
    std::string stats;
    std::string steps;
    auto srow = [&](char const *k, char const *v) {
        if (!stats.empty()) stats += ',';
        stats += "{\"k\":\"";
        esc(stats, k);
        stats += "\",\"v\":\"";
        esc(stats, v);
        stats += "\"}";
    };
    auto srowf = [&](char const *k, float v) {
        char buf[64];
        if (std::fabs(v - (float) (int) v) < 1e-6f)
            std::snprintf(buf, sizeof(buf), "%d", (int) v);
        else
            std::snprintf(buf, sizeof(buf), "%.4g", (double) v);
        srow(k, buf);
    };
    auto step = [&](char const *zh, char const *en) {
        if (!steps.empty()) steps += ',';
        steps += "{\"zh\":\"";
        esc(steps, zh);
        steps += "\",\"en\":\"";
        esc(steps, en);
        steps += "\"}";
    };

    if (std::strcmp(id, "rarity") == 0) {
        for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
            char k[64];
            std::snprintf(k, sizeof(k), "#%u %s / %s", r, rarity_zh(r), rarity_en(r));
            char col[16];
            std::snprintf(col, sizeof(col), "#%06x", RARITY_COLORS[r] & 0xffffffu);
            srow(k, col);
        }
        step("同名英文字段 PETAL_DATA.name 为梯子键", "Same English PETAL_DATA.name keys the ladder");
        step("合成：Craft::petal_at_rarity / craft_result 优先最近同名 ID", "Craft::petal_at_rarity / craft_result prefer nearest same-name ID");
        step("怪掉稀有度带：自身 20% / −1 60% / −2 20%；Unique 怪 50%/35%/15%", "Mob drop rarity: self 20% / −1 60% / −2 20%; Unique mobs 50%/35%/15%");
        step("事件/宝箱：Ultra/Super/Unique 池（TreasureSites::roll_mat_rarity）", "Events/chests: Ultra/Super/Unique pool");
    } else if (std::strcmp(id, "craft") == 0) {
        srowf("合成批量 kCraftBatch", (float) Craft::kCraftBatch);
        srowf("失败最少损耗", (float) Craft::kCraftFailLossMin);
        srowf("失败最多损耗", (float) Craft::kCraftFailLossMax);
        srowf("唯一等级每级属性", Craft::kLevelStatPerLevel);
        srow("唯一成功率公式", "max(0.02, 0.60 × 0.9^level)");
        srowf("唯一 Lv0 成功率", Craft::level_success_chance(0));
        srowf("唯一 Lv10 成功率", Craft::level_success_chance(10));
        srowf("唯一成功率下限", 0.02f);
        for (uint8_t r = 0; r < RarityID::kNumRarities - 1; ++r) {
            char k[80];
            std::snprintf(k, sizeof(k), "成功率 %s→%s", rarity_zh(r), rarity_zh((uint8_t) (r + 1)));
            char v[48];
            float p = Craft::success_chance(r);
            std::snprintf(v, sizeof(v), "%.4g%% (兑换≈%u)", (double) (p * 100.f),
                          (unsigned) Craft::exchange_cost(r));
            srow(k, v);
        }
        step("副槽 / craft_bank 互通材料", "Secondary slots share craft_bank");
        step("托盘放入 5 同瓣 → success_chance(from_rarity) 判定", "Tray 5 same → success_chance(from_rarity)");
        step("成功：写入 rarity+1 同名（petal_at_rarity）；失败：损耗 1–4，其余退回", "Win: next rarity same-name; miss: lose 1–4, refund rest");
        step("唯一：无限等级赌；+5%/级；仅主手第一枚吃加成", "Unique: infinite level gamble; +5%/lv; first primary copy only");
        step("超凡+ 成功进全服播报与排行榜", "Super+ success → broadcast + leaderboard");
    } else if (std::strcmp(id, "loadout") == 0) {
        srowf("基础主槽 kBaseSlots", (float) Talent::kBaseSlots);
        srowf("主槽硬顶 MAX_SLOT_COUNT", (float) MAX_SLOT_COUNT);
        srowf("天赋槽位最多 kMaxSlotRanks", (float) Talent::kMaxSlotRanks);
        srowf("等级每 N 级+1槽（无主生成）", (float) LEVELS_PER_EXTRA_SLOT);
        srow("登录玩家槽位", "Talent::slot_count = min(10, 5 + slots_rank)");
        srow("无主/初始槽位", "loadout_slots_at_level = 5 + level/15");
        srow("配装热键", "K / L 云端预设");
        step("主槽作战 / 副槽背包", "Primary combat / secondary bag");
        step("Spawn/Camera：有 owner 用天赋槽位，否则用等级公式", "Spawn/Camera: talent slots if owned, else level formula");
        step("死亡后仍可拖槽、R 切换、K·L 配装", "After death: drag slots, R swap, K·L presets");
        step("空预设内容进 craft_bank，避免吞瓣", "Empty preset → craft_bank (no petal eat)");
        step("组装界面悬停显示 tip（全数值）", "Assemble UI hover tips with full stats");
    } else if (std::strcmp(id, "combat") == 0) {
        srow("攻击", "左键 / 空格 — InputFlags::kAttacking");
        srow("防御", "右键 / Shift — InputFlags::kDefending");
        srow("反转攻击", "交换攻防方向（全交换）");
        srow("冷却乘算", "Σ petal.extra_reload_factor × Talent::reload_mult");
        srow("月亮", "独立轨道；每 tick 推进角度；攻击可甩出");
        srow("磁铁", "Talent 磁铁半径 + 花瓣 magnet_radius → pickup_drop");
        step("花瓣环绕花心；攻/防改变姿态半径与面向", "Petals orbit flower; attack/defend change pose");
        step("黄金之叶等 extra_reload_factor 与冷却天赋相乘", "Golden Leaf × cooldown talent");
        step("月亮系强制分离 + 独立绕飞", "Moon petals force-separate + own orbit");
        step("磁铁吸入走 pickup_drop（非仅碰撞）", "Magnet uses pickup_drop, not collide-only");
    } else if (std::strcmp(id, "xp") == 0) {
        srowf("1 级基础生命 BASE_HEALTH", BASE_HEALTH);
        srow("生命公式", "100 + 8*(L-1) + 0.08*(L-1)^2");
        srow("体伤公式", "BASE + 0.4*g + 0.0025*g^2（g=L-1）");
        srowf("L1 HP", hp_at_level(1));
        srowf("L50 HP", hp_at_level(50));
        srowf("L100 HP", hp_at_level(100));
        srowf("L1 槽位(无天赋)", (float) loadout_slots_at_level(1));
        srowf("L60 槽位(无天赋)", (float) loadout_slots_at_level(60));
        step("击杀加分：怪 xp × 区域 × 生成稀有度 × 击杀倍率", "Kill score: mob xp × zone × spawn-rarity × kill mult");
        step("score_to_level 累减 score_to_pass_level；高水位存档", "score_to_level walks pass thresholds; high-water saved");
        step("溢出钳 uint32；排行榜同步分数/等级", "Clamp uint32; leaderboard syncs score/level");
    } else if (std::strcmp(id, "talent") == 0) {
        srow("点数", "points_earned = level（每级 1 点）");
        srow("升一档花费", "rank_cost(r) = 2*r − 1");
        srowf("体魄 / 级", 50.f);
        srowf("转速 / 级", 0.4f);
        srowf("射程 / 级", 8.f);
        srowf("磁铁半径 / 级", 200.f);
        srowf("瓣甲倍率 / 级", 0.08f);
        srow("冷却", "每级 ×0.97（连乘）");
        srowf("普通天赋满级档", (float) Talent::kMaxGradeRank);
        srowf("槽位天赋满级档", (float) Talent::kMaxSlotRanks);
        srow("等级对档位上限", "max_rank ≈ 1 + level/10（槽位除外）");
        step("TalentID：体魄/转速/射程/冷却/磁铁/瓣甲/槽位", "TalentID: Health/Rot/Range/CD/Magnet/Armor/Slots");
        step("can_set_rank 检查等级上限与剩余点数", "can_set_rank checks level cap + free points");
        step("槽位 = min(10, 5 + slots_rank)；另有等级赠槽", "Slots = min(10, 5+rank); level also grants slots");
    } else if (std::strcmp(id, "drop") == 0) {
        srow("怪掉稀有度带", "自身 20% / −1 60% / −2 20%");
        srow("普怪基础堆数", "2 + 概率追加");
        srow("追加概率", "0.72；Unusual+0.55；Rare+0.45；Epic+0.40；Legendary+0.35");
        srow("Boss 堆数", "+4 保底，再 70% / 45%");
        srow("种子权重", "compute_mob_drop_weights + Craft::mob_loot_weights");
        srow("Chase 特殊", "月/世界树/金叶等扁平份额后再倾斜");
        srow("事件普通档", "Ultra 55% / Super 35% / Unique 10%");
        srow("事件传说档 Unique", "8%");
        srow("事件高难档 Unique", "12%");
        srow("仪式档 Unique", "14%");
        srow("探索全池占比", "88%（其余地图主题材料）");
        srow("仪式全池占比", "75%");
        step("Death：roll 种子权重 → petal_at_rarity(seed, drop_r)", "Death: weight seed → petal_at_rarity");
        step("事件/宝箱：roll_mat_rarity(tier) → 全池或地图材料", "Events: roll_mat_rarity → pool or map mat");
        step("三倍加成开启时额外翻倍堆（Unique 除外）", "Triple bonus doubles piles (excludes Unique)");
    } else if (std::strcmp(id, "world") == 0) {
        srow("世界事件槽位", "每图 12 × 8 图 = 96");
        srow("传说触摸奖励", "120 币 + 5 传说档残片");
        srow("逃离奖励", "280 币 + 6");
        srow("睡觉怪奖励", "220 币 + 5");
        srow("伏击奖励", "240 币 + 5");
        srow("幻影箱奖励", "100 币 + 4");
        srow("列高奖励", "360 币 + 7");
        srow("限时三印奖励", "340 币 + 7");
        srow("护送终点", "320 币 + 6");
        srow("三印探索", "400 币 + 7（每图一次）");
        srow("道具 zone", "0xFF（不被野怪缓清误删）");
        step("WorldEvent / WorldExplore / Treasure 三套系统", "WorldEvent / WorldExplore / Treasure");
        step("ThemeArt::resolve(mob,name,map) 决定程序美术", "ThemeArt::resolve picks procedural art");
        step("点「代码流程」或「世界介绍」看全部变体图文", "See Processes / Maps for full illustrated roster");
    } else if (std::strcmp(id, "social") == 0) {
        srow("聊天频道", "附近 / 队伍 / 公会 / 私聊 / 全服");
        srow("chatlog", "持久化聊天记录 /api/chatlog");
        srow("broadcasts", "击杀 / 高级合成 / 公告 /api/broadcasts");
        srow("组队", "共享击杀经验");
        srow("排行榜维度", "在线时长、等级、发言、Boss、高级合成");
        step("Chat.cc 写入；Live 看板拉取历史", "Chat.cc persists; Live console queries history");
        step("公会/组队实体 TeamManager / Party / Guild", "Guild/Party via TeamManager");
    } else if (std::strcmp(id, "bonus") == 0) {
        srow("每日加成发放", "3600 秒（1 小时）");
        srow("离线累计上限", "21600 秒（6 小时）");
        srow("效果", "开启后击杀掉落额外翻倍堆（Unique 除外）");
        srow("需登录账号", "未登录不累计");
        step("AccountStore 登录发 daily=3600，离线累加封顶 21600", "Login daily=3600; offline accrue cap 21600");
        step("Client 手动开启；Death 检测 daily/offline 秒数", "Client toggles; Death checks remaining seconds");
        step("消耗优先扣 daily，再扣 offline", "Spend daily first, then offline");
    }

    out += ",\"stats\":[";
    out += stats;
    out += "],\"steps\":[";
    out += steps;
    out += "]";
}

static void emit_petal_attrs(std::string &out, PetalAttributes const &a) {
    out += ",\"attrs\":{";
    uint8_t f = 1;
    jkv_f(out, "clump_radius", a.clump_radius, f);
    jkv_f(out, "secondary_reload", a.secondary_reload, f);
    jkv_f(out, "constant_heal", a.constant_heal, f);
    jkv_f(out, "burst_heal", a.burst_heal, f);
    jkv_f(out, "mass", a.mass, f);
    jkv_f(out, "armor", a.armor, f);
    jkv_f(out, "poison_armor", a.poison_armor, f);
    jkv_f(out, "dandelion_inflict_seconds", a.dandelion_inflict_seconds, f);
    jkv_f(out, "slow_inflict_seconds", a.slow_inflict_seconds, f);
    jkv_f(out, "vision_factor", a.vision_factor, f);
    jkv_f(out, "extra_body_damage", a.extra_body_damage, f);
    jkv_f(out, "extra_rotation_speed", a.extra_rotation_speed, f);
    jkv_f(out, "extra_range", a.extra_range, f);
    jkv_f(out, "extra_health", a.extra_health, f);
    jkv_f(out, "damage_reflection", a.damage_reflection, f);
    jkv_f(out, "extra_damage_factor", a.extra_damage_factor, f);
    jkv_f(out, "extra_reload_factor", a.extra_reload_factor, f);
    jkv_f(out, "move_speed_factor", a.move_speed_factor, f);
    jkv_f(out, "magnet_radius", a.magnet_radius, f);
    jkv_f(out, "aura_heal", a.aura_heal, f);
    jkv_f(out, "aura_radius", a.aura_radius, f);
    jkv_f(out, "aura_duration", a.aura_duration, f);
    jkv_f(out, "poison_dps", a.poison_damage.damage, f);
    jkv_f(out, "poison_time", a.poison_damage.time, f);
    jkv_f(out, "burn_dps", a.burn_damage.damage, f);
    jkv_f(out, "burn_time", a.burn_damage.time, f);
    jkv_f(out, "armor_penetration", a.armor_penetration, f);
    jkv_f(out, "bonus_vs_poisoned", a.bonus_vs_poisoned, f);
    jkv_f(out, "stack_damage", a.stack_damage, f);
    jkv_f(out, "attraction_radius", a.attraction_radius, f);
    jkv_u(out, "fixed_orbit_range", a.fixed_orbit_range, f);
    jkv_f(out, "crit_chance", a.crit_chance, f);
    jkv_f(out, "crit_factor", a.crit_factor, f);
    jkv_f(out, "luck", a.luck, f);
    jkv_f(out, "armor_shred", a.armor_shred, f);
    jkv_f(out, "armor_shred_seconds", a.armor_shred_seconds, f);
    jkv_f(out, "body_radius_bonus", a.body_radius_bonus, f);
    jkv_f(out, "heal_amp", a.heal_amp, f);
    jkv_f(out, "damage_absorb", a.damage_absorb, f);
    jkv_f(out, "knockback_taken", a.knockback_taken, f);
    jkv_f(out, "solo_damage_factor", a.solo_damage_factor, f);
    jkv_f(out, "petal_evasion", a.petal_evasion, f);
    jkv_f(out, "flower_evasion", a.flower_evasion, f);
    jkv_f(out, "nazar_mitigation", a.nazar_mitigation, f);
    jkv_u(out, "nazar_charges", a.nazar_charges, f);
    jkv_f(out, "petal_hp_factor", a.petal_hp_factor, f);
    jkv_f(out, "petal_repair", a.petal_repair, f);
    jkv_f(out, "clay_max_damage", a.clay_max_damage, f);
    jkv_f(out, "tomato_max_damage", a.tomato_max_damage, f);
    jkv_f(out, "domino_max_damage", a.domino_max_damage, f);
    jkv_f(out, "vs_projectile_factor", a.vs_projectile_factor, f);
    jkv_f(out, "lightning_damage", a.lightning_damage, f);
    jkv_u(out, "lightning_chains", a.lightning_chains, f);
    jkv_u(out, "battery_charges", a.battery_charges, f);
    jkv_f(out, "meteor_size_factor", a.meteor_size_factor, f);
    jkv_f(out, "secondary_cdr_factor", a.secondary_cdr_factor, f);
    jkv_f(out, "damage_delay_seconds", a.damage_delay_seconds, f);
    jkv_f(out, "on_hit_heal_ratio", a.on_hit_heal_ratio, f);
    jkv_f(out, "hit_interval_seconds", a.hit_interval_seconds, f);
    jkv_u(out, "phase_through", a.phase_through, f);
    jkv_f(out, "aggro_shrink", a.aggro_shrink, f);
    jkv_u(out, "ankh_mark", a.ankh_mark, f);
    jkv_u(out, "shovel_burrow", a.shovel_burrow, f);
    jkv_f(out, "death_delay_seconds", a.death_delay_seconds, f);
    jkv_f(out, "totem_heal", a.totem_heal, f);
    jkv_f(out, "totem_seconds", a.totem_seconds, f);
    jkv_f(out, "root_armor", a.root_armor, f);
    jkv_f(out, "root_interval", a.root_interval, f);
    jkv_u(out, "root_max_stacks", a.root_max_stacks, f);
    jkv_u(out, "wax_block", a.wax_block, f);
    jkv_f(out, "mana_gen", a.mana_gen, f);
    jkv_f(out, "mana_cap", a.mana_cap, f);
    jkv_f(out, "mana_absorb_per", a.mana_absorb_per, f);
    jkv_f(out, "mana_lightning_per", a.mana_lightning_per, f);
    jkv_f(out, "mana_spawn_cost", a.mana_spawn_cost, f);
    jkv_u(out, "card_random_stats", a.card_random_stats, f);
    jkv_u(out, "mimic_left", a.mimic_left, f);
    jkv_u(out, "compass", a.compass, f);
    jkv_u(out, "blood_sacrifice", a.blood_sacrifice, f);
    jkv_u(out, "champion_crown", a.champion_crown, f);
    jkv_u(out, "soul_splitter", a.soul_splitter, f);
    jkv_u(out, "paranormal", a.paranormal, f);
    jkv_u(out, "corruption", a.corruption, f);
    jkv_u(out, "dark_mark", a.dark_mark, f);
    jkv_f(out, "overheal_to_shield", a.overheal_to_shield, f);
    jkv_f(out, "privet_extra_poison", a.privet_extra_poison, f);
    jkv_u(out, "powder_max_rarity", a.powder_max_rarity, f);
    jkv_f(out, "bonus_vs_slowed", a.bonus_vs_slowed, f);
    jkv_u(out, "hit_cycle", a.hit_cycle, f);
    jkv_f(out, "hit_cycle_factor", a.hit_cycle_factor, f);
    jkv_f(out, "snare_seconds", a.snare_seconds, f);
    jkv_f(out, "splash_damage", a.splash_damage, f);
    jkv_f(out, "splash_radius", a.splash_radius, f);
    jkv_f(out, "execute_threshold", a.execute_threshold, f);
    jkv_f(out, "execute_factor", a.execute_factor, f);
    jkv_f(out, "taunt_radius", a.taunt_radius, f);
    jkv_f(out, "on_hit_heal", a.on_hit_heal, f);
    jkv_f(out, "radial_pulse_damage", a.radial_pulse_damage, f);
    jkv_f(out, "radial_pulse_radius", a.radial_pulse_radius, f);
    jkv_f(out, "aura_slow_seconds", a.aura_slow_seconds, f);
    jkv_u(out, "defend_only", a.defend_only, f);
    jkv_f(out, "icon_angle", a.icon_angle, f);
    jkv_u(out, "split_projectile", a.split_projectile, f);
    jkv_s(out, "rotation_style", rot_name(a.rotation_style), f);
    jkv_u(out, "spawns", a.spawns, f);
    jkv_u(out, "spawn_count", a.spawn_count, f);
    jkv_s(out, "equipment", equip_name(a.equipment), f);
    out += '}';
}

static void emit_u8_array(std::string &out, std::vector<uint8_t> const &v) {
    out += '[';
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) out += ',';
        out += std::to_string((unsigned) v[i]);
    }
    out += ']';
}

static void emit_petal(std::string &out, uint32_t id,
                       std::unordered_map<std::string, std::vector<uint32_t>> const &ladders,
                       std::vector<uint8_t> const &maps) {
    PetalData const &p = PETAL_DATA[id];
    char const *en = p.name ? p.name : "";
    out += '{';
    uint8_t f = 1;
    jkv_u(out, "id", id, f);
    jkv_s(out, "name", en, f);
    jkv_s(out, "name_zh", LIVE_PETAL_NAME_ZH[id] ? LIVE_PETAL_NAME_ZH[id] : en, f);
    jkv_s(out, "desc", p.description ? p.description : "", f);
    jkv_s(out, "desc_zh", LIVE_PETAL_DESC_ZH[id] ? LIVE_PETAL_DESC_ZH[id] : "", f);
    jkv_u(out, "rarity", p.rarity, f);
    jkv_s(out, "rarity_zh", rarity_zh(p.rarity), f);
    jkv_s(out, "rarity_en", rarity_en(p.rarity), f);
    jkv_f(out, "hp", p.health, f);
    jkv_f(out, "dmg", p.damage, f);
    jkv_f(out, "radius", p.radius, f);
    jkv_f(out, "reload", p.reload, f);
    jkv_u(out, "count", p.count, f);
    jkv_f(out, "drop_weight_mult", Craft::drop_weight_mult((PetalID::T) id), f);
    jkv_u(out, "chase", is_chase_loot_petal((PetalID::T) id) ? 1u : 0u, f);
    jkv_u(out, "can_upgrade", Craft::can_upgrade_petal((PetalID::T) id) ? 1u : 0u, f);
    {
        PetalID::T next = PetalID::kNone;
        if (p.rarity + 1 < RarityID::kNumRarities)
            next = Craft::petal_at_rarity((PetalID::T) id, (uint8_t) (p.rarity + 1));
        jkv_u(out, "craft_next", (unsigned) next, f);
    }
    emit_petal_attrs(out, p.attributes);
    out += ",\"maps\":";
    emit_u8_array(out, maps);
    out += ",\"ladder\":[";
    auto it = ladders.find(en);
    if (it != ladders.end()) {
        for (size_t i = 0; i < it->second.size(); ++i) {
            if (i) out += ',';
            out += std::to_string(it->second[i]);
        }
    }
    out += "]}";
}

static void emit_mob_drops(std::string &out, uint32_t mob_id, uint8_t view_rarity) {
    if (mob_id >= MobID::kNumMobs) { out += "[]"; return; }
    auto const &drops = MOB_DATA[mob_id].drops;
    uint32_t n = drops.size();
    if (n > MAX_DROPS_PER_MOB) n = MAX_DROPS_PER_MOB;
    float weights[MAX_DROPS_PER_MOB] = {};
    float total = Craft::mob_loot_weights((MobID::T) mob_id, view_rarity, weights);
    out += '[';
    for (uint32_t i = 0; i < n; ++i) {
        if (i) out += ',';
        PetalID::T seed = drops[i];
        PetalID::T shown = Craft::petal_at_rarity(seed, view_rarity);
        if (shown == PetalID::kNone || shown >= PetalID::kNumPetals
            || is_chase_loot_petal(seed))
            shown = seed;
        if (shown >= PetalID::kNumPetals) shown = PetalID::kNone;
        float pct = (total > 0.f) ? (weights[i] / total * 100.f) : 0.f;
        char const *en = (shown < PetalID::kNumPetals && PETAL_DATA[shown].name)
            ? PETAL_DATA[shown].name : "";
        uint8_t rar = (shown < PetalID::kNumPetals) ? PETAL_DATA[shown].rarity : 0;
        out += "{\"seed\":";
        out += std::to_string((unsigned) seed);
        out += ",\"petal\":";
        out += std::to_string((unsigned) shown);
        out += ",\"name\":\"";
        esc(out, en);
        out += "\",\"name_zh\":\"";
        esc(out, (shown < PetalID::kNumPetals && LIVE_PETAL_NAME_ZH[shown])
                ? LIVE_PETAL_NAME_ZH[shown] : en);
        out += "\",\"rarity\":";
        out += std::to_string(rar);
        out += ",\"rarity_zh\":\"";
        out += rarity_zh(rar);
        out += "\",\"weight\":";
        jnum(out, weights[i]);
        out += ",\"pct\":";
        jnum(out, pct);
        out += ",\"chase\":";
        out += is_chase_loot_petal(seed) ? '1' : '0';
        out += '}';
    }
    out += ']';
}

// Mob IDs that actually spawn on each map (MapGen zones + boss anchor).
static void collect_map_mobs(std::vector<std::vector<uint32_t>> &by_map,
                             std::vector<std::vector<uint8_t>> &mob_to_maps) {
    by_map.assign(MapID::kNumMaps, {});
    mob_to_maps.assign(MobID::kNumMobs, {});
    for (uint8_t mid = 0; mid < MapID::kNumMaps; ++mid) {
        MapGen::ensure(mid);
        std::unordered_set<uint32_t> seen;
        MapZones const &zones = MapGen::zones(mid);
        for (uint32_t zi = 0; zi < zones.size(); ++zi) {
            auto const &spawns = zones[zi].spawns;
            for (uint32_t si = 0; si < spawns.size(); ++si) {
                uint32_t id = (uint32_t) spawns[si].id;
                if (id == 0 || id >= MobID::kNumMobs) continue;
                if (seen.insert(id).second) by_map[mid].push_back(id);
            }
        }
        MapGen::BossAnchor const &boss = MapGen::boss_anchor(mid);
        if (boss.mob_id && boss.mob_id < MobID::kNumMobs
            && seen.insert(boss.mob_id).second)
            by_map[mid].push_back(boss.mob_id);
        std::sort(by_map[mid].begin(), by_map[mid].end());
        for (uint32_t id : by_map[mid])
            mob_to_maps[id].push_back(mid);
    }
}

static void emit_mob(std::string &out, uint32_t id,
                     std::vector<std::vector<uint8_t>> const &mob_to_maps) {
    MobData const &m = MOB_DATA[id];
    char const *en = m.name ? m.name : "";
    out += '{';
    uint8_t f = 1;
    jkv_u(out, "id", id, f);
    jkv_s(out, "name", en, f);
    jkv_s(out, "name_zh", LIVE_MOB_NAME_ZH[id] ? LIVE_MOB_NAME_ZH[id] : en, f);
    jkv_s(out, "desc", m.description ? m.description : "", f);
    jkv_s(out, "desc_zh", LIVE_MOB_DESC_ZH[id] ? LIVE_MOB_DESC_ZH[id] : "", f);
    jkv_u(out, "rarity", m.rarity, f);
    jkv_s(out, "rarity_zh", rarity_zh(m.rarity), f);
    jkv_s(out, "rarity_en", rarity_en(m.rarity), f);
    jkv_f(out, "hp_lo", m.health.lower, f);
    jkv_f(out, "hp_hi", m.health.upper, f);
    jkv_f(out, "dmg", m.damage, f);
    jkv_f(out, "radius_lo", m.radius.lower, f);
    jkv_f(out, "radius_hi", m.radius.upper, f);
    jkv_u(out, "xp", m.xp, f);
    jkv_f(out, "aggro", m.attributes.aggro_radius, f);
    jkv_u(out, "segments", m.attributes.segments, f);
    jkv_u(out, "stationary", m.attributes.stationary, f);
    jkv_f(out, "poison_dps", m.attributes.poison_damage.damage, f);
    jkv_f(out, "poison_time", m.attributes.poison_damage.time, f);
    jkv_u(out, "boss", is_boss_mob((MobID::T) id) ? 1u : 0u, f);
    out += ",\"maps\":";
    if (id < mob_to_maps.size()) emit_u8_array(out, mob_to_maps[id]);
    else out += "[]";
    out += ",\"drops\":";
    emit_mob_drops(out, id, m.rarity);
    // Full rarity ladder of drop tables for balance editing
    out += ",\"drops_by_rarity\":[";
    for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
        if (r) out += ',';
        out += "{\"rarity\":";
        out += std::to_string(r);
        out += ",\"rarity_zh\":\"";
        out += rarity_zh(r);
        out += "\",\"entries\":";
        emit_mob_drops(out, id, r);
        out += '}';
    }
    out += "]}";
}

} // namespace

std::string Server::live_codex_json() {
    static std::string cached;
    if (!cached.empty()) return cached;

    std::string out;
    out.reserve(8 * 1024 * 1024);

    // Global rules (mirrors Death.cc / Craft.cc / TreasureSites) for designers
    out += "{"
        "\"rules\":{"
          "\"mob_drop_rarity_bands\":["
            "{\"offset\":0,\"pct\":20,\"zh\":\"与怪同稀有度\",\"en\":\"same as mob\"},"
            "{\"offset\":-1,\"pct\":60,\"zh\":\"低 1 阶\",\"en\":\"mob rarity −1\"},"
            "{\"offset\":-2,\"pct\":20,\"zh\":\"低 2 阶\",\"en\":\"mob rarity −2\"}"
          "],"
          "\"mob_drop_pile\":{"
            "\"base_rolls\":2,"
            "\"extra_chances\":[0.72,0.55,0.45,0.40,0.35],"
            "\"zh\":\"基础 2 堆，再按怪稀有度追加；Boss 另有保底加成\","
            "\"en\":\"2 base piles then rarity extras; bosses get guaranteed extras\""
          "},"
          "\"craft_success\":[1,0.72,0.36,0.18,0.08,0.03,0.016,0.008],"
          "\"event_rarity_normal\":{\"ultra\":0.55,\"super\":0.35,\"unique\":0.10},"
          "\"event_full_pool_pct\":0.88,"
          "\"notes_zh\":\"怪掉种子权重见各 mob.drops；稀有度再按 20/60/20 带解析到同名阶梯。\","
          "\"notes_en\":\"Seed weights in each mob.drops; rarity then resolved via 20/60/20 onto the same-name ladder.\""
        "},";

    out += "\"rarities\":[";
    for (uint8_t r = 0; r < RarityID::kNumRarities; ++r) {
        if (r) out += ',';
        char col[16];
        std::snprintf(col, sizeof(col), "#%06x", RARITY_COLORS[r] & 0xffffffu);
        out += "{\"id\":";
        out += std::to_string(r);
        out += ",\"name_zh\":\"";
        out += rarity_zh(r);
        out += "\",\"name_en\":\"";
        out += rarity_en(r);
        out += "\",\"color\":\"";
        out += col;
        out += "\"}";
    }

    out += "],\"maps\":[";
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
        if (i) out += ',';
        MapIntro const &m = kCodexMaps[i];
        char col[16];
        std::snprintf(col, sizeof(col), "#%06x", m.color & 0xffffffu);
        out += "{\"id\":";
        out += std::to_string(i);
        out += ",\"name_zh\":\"";
        esc(out, m.zh);
        out += "\",\"name_en\":\"";
        esc(out, m.en);
        out += "\",\"tagline\":\"";
        esc(out, m.tagline_zh);
        out += "\",\"tagline_en\":\"";
        esc(out, m.tagline_en);
        out += "\",\"body\":\"";
        esc(out, m.body_zh);
        out += "\",\"body_en\":\"";
        esc(out, m.body_en);
        out += "\",\"color\":\"";
        out += col;
        out += "\"}";
    }

    out += "],\"mechanics\":[";
    for (size_t i = 0; i < sizeof(MECHS) / sizeof(MECHS[0]); ++i) {
        if (i) out += ',';
        MechChapter const &c = MECHS[i];
        out += "{\"id\":\"";
        esc(out, c.id);
        out += "\",\"title\":\"";
        esc(out, c.title_zh);
        out += "\",\"title_en\":\"";
        esc(out, c.title_en);
        out += "\",\"summary\":\"";
        esc(out, c.summary_zh);
        out += "\",\"summary_en\":\"";
        esc(out, c.summary_en);
        out += "\",\"detail\":\"";
        esc(out, c.detail_zh);
        out += "\",\"detail_en\":\"";
        esc(out, c.detail_en);
        out += "\"";
        emit_mech_payload(out, c.id);
        out += "}";
    }

    out += "],\"art_kinds\":[";
    for (size_t i = 0; i < sizeof(ARTS) / sizeof(ARTS[0]); ++i) {
        if (i) out += ',';
        out += "{\"id\":\"";
        esc(out, ARTS[i].id);
        out += "\",\"name_zh\":\"";
        esc(out, ARTS[i].zh);
        out += "\",\"name_en\":\"";
        esc(out, ARTS[i].en);
        out += "\",\"role\":\"";
        esc(out, ARTS[i].role_zh);
        out += "\",\"role_en\":\"";
        esc(out, ARTS[i].role_en);
        out += "\"}";
    }

    out += "],\"processes\":[";
    for (size_t i = 0; i < sizeof(kCodexProcesses) / sizeof(kCodexProcesses[0]); ++i) {
        if (i) out += ',';
        CodexProcess const &p = kCodexProcesses[i];
        out += "{\"id\":\"";
        esc(out, p.id);
        out += "\",\"title\":\"";
        esc(out, p.title_zh);
        out += "\",\"title_en\":\"";
        esc(out, p.title_en);
        out += "\",\"detail\":\"";
        esc(out, p.detail_zh);
        out += "\",\"detail_en\":\"";
        esc(out, p.detail_en);
        out += "\"}";
    }

    out += "],\"world_entities\":[";
    for (size_t i = 0; i < sizeof(kCodexWorld) / sizeof(kCodexWorld[0]); ++i) {
        if (i) out += ',';
        CodexWorldEnt const &e = kCodexWorld[i];
        out += "{\"map_id\":";
        out += std::to_string(e.map_id);
        out += ",\"visual\":";
        out += std::to_string(e.visual);
        out += ",\"art_id\":\"";
        esc(out, e.art_id);
        out += "\",\"group\":\"";
        esc(out, e.group);
        out += "\",\"kind\":\"";
        esc(out, e.kind);
        out += "\",\"name_zh\":\"";
        esc(out, e.name_zh);
        out += "\",\"name_en\":\"";
        esc(out, e.name_en);
        out += "\",\"flow_zh\":\"";
        esc(out, e.flow_zh);
        out += "\",\"flow_en\":\"";
        esc(out, e.flow_en);
        out += "\",\"lore_zh\":\"";
        esc(out, e.lore_zh);
        out += "\",\"lore_en\":\"";
        esc(out, e.lore_en);
        out += "\",\"alert_zh\":\"";
        esc(out, e.alert_zh);
        out += "\",\"awake_zh\":\"";
        esc(out, e.awake_zh);
        out += "\"}";
    }

    // Ladder index: English name → sorted petal ids by rarity then id
    std::unordered_map<std::string, std::vector<uint32_t>> ladders;
    for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
        char const *en = PETAL_DATA[id].name;
        if (en == nullptr || !*en) continue;
        ladders[en].push_back(id);
    }
    for (auto &kv : ladders) {
        auto &v = kv.second;
        std::sort(v.begin(), v.end(), [](uint32_t a, uint32_t b) {
            if (PETAL_DATA[a].rarity != PETAL_DATA[b].rarity)
                return PETAL_DATA[a].rarity < PETAL_DATA[b].rarity;
            return a < b;
        });
    }

    std::vector<std::vector<uint32_t>> map_mobs;
    std::vector<std::vector<uint8_t>> mob_to_maps;
    collect_map_mobs(map_mobs, mob_to_maps);

    // Petal maps = maps of mobs that list this seed (or any same-name ladder mate).
    std::vector<std::unordered_set<uint8_t>> petal_map_set(PetalID::kNumPetals);
    for (uint32_t mid = 0; mid < MobID::kNumMobs; ++mid) {
        if (mob_to_maps[mid].empty()) continue;
        auto const &drops = MOB_DATA[mid].drops;
        for (uint32_t di = 0; di < drops.size(); ++di) {
            uint32_t seed = (uint32_t) drops[di];
            if (seed == 0 || seed >= PetalID::kNumPetals) continue;
            char const *en = PETAL_DATA[seed].name;
            if (en == nullptr || !*en) {
                for (uint8_t m : mob_to_maps[mid]) petal_map_set[seed].insert(m);
                continue;
            }
            auto lit = ladders.find(en);
            if (lit == ladders.end()) {
                for (uint8_t m : mob_to_maps[mid]) petal_map_set[seed].insert(m);
                continue;
            }
            for (uint32_t pid : lit->second)
                for (uint8_t m : mob_to_maps[mid]) petal_map_set[pid].insert(m);
        }
    }

    out += "],\"map_spawns\":[";
    for (uint8_t mid = 0; mid < MapID::kNumMaps; ++mid) {
        if (mid) out += ',';
        out += "{\"map_id\":";
        out += std::to_string(mid);
        out += ",\"mobs\":[";
        for (size_t i = 0; i < map_mobs[mid].size(); ++i) {
            if (i) out += ',';
            out += std::to_string(map_mobs[mid][i]);
        }
        out += "]}";
    }

    out += "],\"petals\":[";
    {
        uint8_t first_p = 1;
        for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
            char const *en = PETAL_DATA[id].name;
            if (en == nullptr || !*en) continue;
            if (!first_p) out += ',';
            first_p = 0;
            std::vector<uint8_t> maps(petal_map_set[id].begin(), petal_map_set[id].end());
            std::sort(maps.begin(), maps.end());
            emit_petal(out, id, ladders, maps);
        }
    }

    out += "],\"mobs\":[";
    {
        uint8_t first_m = 1;
        for (uint32_t id = 0; id < (uint32_t) MobID::kNumMobs; ++id) {
            char const *en = MOB_DATA[id].name;
            if (en == nullptr || !*en) continue;
            if (!first_m) out += ',';
            first_m = 0;
            emit_mob(out, id, mob_to_maps);
        }
    }

    out += "]}";
    cached = std::move(out);
    return cached;
}

std::string Server::live_codex_gate_json() {
    static std::string cached;
    if (!cached.empty()) return cached;

    std::string out;
    out.reserve(16 * 1024);
    out += "{\"maps\":[";
    for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
        if (i) out += ',';
        MapIntro const &m = kCodexMaps[i];
        char col[16];
        std::snprintf(col, sizeof(col), "#%06x", m.color & 0xffffffu);
        out += "{\"id\":";
        out += std::to_string(i);
        out += ",\"name_zh\":\"";
        esc(out, m.zh);
        out += "\",\"name_en\":\"";
        esc(out, m.en);
        out += "\",\"tagline\":\"";
        esc(out, m.tagline_zh);
        out += "\",\"color\":\"";
        out += col;
        out += "\"}";
    }
    out += "],\"mechanics\":[";
    for (size_t i = 0; i < sizeof(MECHS) / sizeof(MECHS[0]); ++i) {
        if (i) out += ',';
        MechChapter const &c = MECHS[i];
        out += "{\"id\":\"";
        esc(out, c.id);
        out += "\",\"title\":\"";
        esc(out, c.title_zh);
        out += "\",\"summary\":\"";
        esc(out, c.summary_zh);
        out += "\"}";
    }
    out += "]}";
    cached = std::move(out);
    return cached;
}
