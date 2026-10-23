#include <Server/Codex.hh>

#include <Shared/Craft.hh>
#include <Shared/RarityScale.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/MobDescZh.inc>
#include <Shared/MobNameZh.inc>
#include <Shared/PetalDescZh.inc>
#include <Shared/PetalNameZh.inc>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
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
    { "chest", "宝箱", "Loot chest", "走近开启的 Yahaha 宝箱", "Walk-up Yahaha chests" },
    { "mirage", "幻影箱", "Mirage chest", "半透明诱饵箱", "Translucent decoy chests" },
    { "flee", "逃离事件", "Flee event", "受惊逃窜的灌木/沙丘/海藻等", "Spooked fleeing bush / dune / kelp…" },
    { "height", "列高事件", "Height event", "蘑菇/仙人掌/贝壳等高度序列", "Ordered mushrooms / cactus / shells…" },
    { "lore", "传说印记", "Lore mark", "种子、石碑、足迹等轻叙事点", "Seeds, steles, footprints — light lore" },
};

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

static void emit_petal(std::string &out, uint32_t id,
                       std::unordered_map<std::string, std::vector<uint32_t>> const &ladders) {
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
    auto const &drops = MOB_DATA[mob_id].drops;
    float weights[MAX_DROPS_PER_MOB] = {};
    float total = Craft::mob_loot_weights((MobID::T) mob_id, view_rarity, weights);
    out += '[';
    for (uint32_t i = 0; i < drops.size(); ++i) {
        if (i) out += ',';
        PetalID::T seed = drops[i];
        PetalID::T shown = Craft::petal_at_rarity(seed, view_rarity);
        if (shown == PetalID::kNone || is_chase_loot_petal(seed))
            shown = seed;
        float pct = (total > 0.f) ? (weights[i] / total * 100.f) : 0.f;
        out += "{\"seed\":";
        out += std::to_string((unsigned) seed);
        out += ",\"petal\":";
        out += std::to_string((unsigned) shown);
        out += ",\"name\":\"";
        esc(out, PETAL_DATA[shown].name ? PETAL_DATA[shown].name : "");
        out += "\",\"name_zh\":\"";
        esc(out, LIVE_PETAL_NAME_ZH[shown] ? LIVE_PETAL_NAME_ZH[shown]
                : (PETAL_DATA[shown].name ? PETAL_DATA[shown].name : ""));
        out += "\",\"rarity\":";
        out += std::to_string(PETAL_DATA[shown].rarity);
        out += ",\"rarity_zh\":\"";
        out += rarity_zh(PETAL_DATA[shown].rarity);
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

static void emit_mob(std::string &out, uint32_t id) {
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
        out += "\"}";
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

    out += "],\"petals\":[";
    {
        uint8_t first_p = 1;
        for (uint32_t id = 1; id < (uint32_t) PetalID::kNumPetals; ++id) {
            char const *en = PETAL_DATA[id].name;
            if (en == nullptr || !*en) continue;
            if (!first_p) out += ',';
            first_p = 0;
            emit_petal(out, id, ladders);
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
            emit_mob(out, id);
        }
    }

    out += "]}";
    cached = std::move(out);
    return cached;
}
