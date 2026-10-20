#include <Client/I18n.hh>

#include <Client/Game.hh>
#include <Client/StaticData.hh>

#include <Shared/StaticData.hh>

#include <format>

namespace I18n {
    uint8_t use_chinese = 1;
}

#include "I18nData.inc"

static char const *const RARITY_ZH[RarityID::kNumRarities] = {
    "普通", "罕见", "稀有", "史诗", "传奇", "神话", "独特", "超凡", "究极"
};

static char const *const ZONE_ZH[] = {
    "花园", "沙漠", "海洋", "雪原", "丛林", "蚁穴", "火山", "王城"
};

static char const *const MAP_EN[] = {
    "Garden", "Desert", "Ocean", "Snow", "Jungle", "Ant Hell", "Volcano", "Royal Capital"
};

static char const *const ROYAL_ZONE_ZH[] = {
    "外城防线", "王城街区", "皇家花园", "皇家兵营", "王城废墟", "王宫核心"
};

static char const *const EN[I18n::kCount] = {
    "Connecting...",
    "This pretty little flower is called...",
    "Spawn",
    "(or press ENTER to spawn)",
    "open-source florr.io pvp clone",
    "How to play",
    "Use mouse to move",
    "Left click to attack",
    "Right click to defend",
    "Use WASD or arrow keys to move",
    "SPACE to attack",
    "SHIFT to defend",
    "You will respawn at level {}",
    " with:",
    "Settings",
    "Petals",
    "Mobs",
    "Changelog",
    "Keyboard movement",
    "Movement helper",
    "Debug stats",
    "Made by 灰烬 (aqiangei) · ashflor",
    "Based on gardn / bismuth (trigonal-bacon) · Assets: M28 et al.",
    "Chinese UI",
    "Stats",
    "Score: {}",
    "Level: {}",
    "Petal Gallery",
    "Monster Bestiary",
    "Leaderboard",
    "Continue",
    "You were killed by",
    "a mysterious entity",
    "(or press ENTER to continue)",
    "Health:",
    "Damage:",
    "Poison:",
    "XP:",
    "Armor:",
    "Damage Reflection:",
    "Heal:",
    "Flower Health:",
    "Body Damage:",
    "Poison Armor:",
    "Rotation Speed:",
    "Contents:",
    "Spawns:",
    "Extra Vision:",
    "Attack Range:",
    "Extra Damage:",
    "Extra Reload:",
    "Move Speed:",
    "Minimap",
    "MOVE RIGHT →",
    "You are overleveled for this zone",
    "You will be unable to attack",
    "Drop",
    "Lvl {} Flower",
    "Extra petal slot at level {}",
    "Unnamed",
    "Move mouse here to disable movement",
    "You can also use [Q] and [E] to cycle secondaries",
    "Failed to show debug stats",
    "tick: {:.1f}/{:.1f}/{:.1f} ms (min/avg/max)",
    "frame: {:.1f}/{:.1f}/{:.1f} ms (min/avg/max) - {:.1f} fps",
    "Disconnected with code {}",
    "Disconnected with code {} (\"{}\")",
    "Older changelog entries not available",
    "Menu",
    "Resume",
    "Press ESC to close",
    "Username",
    "Password",
    "Login",
    "Register",
    "Logout",
    "Logged in",
    "Username must be 3-16 letters, numbers or _",
    "Password must be 4-32 characters",
    "Username already exists",
    "Account not found",
    "Wrong password",
    "This account is already online",
    "Logged out",
    "Not connected",
    "Logged in: {}  Lv.{}",
    "Guest mode (register to keep current progress)",
    "Click to chat",
    "Return to the title screen?",
    "Leave",
    "Cancel",
    "Invite nearby",
    "Leave party",
    "Party",
    "Not in a party",
    "Teammates can revive you (stand by corpse 3s)",
    "Reviving... {}%",
    "Nearby",
    "Party",
    "Guild",
    "Invite to party",
    "Invite to guild",
    "Private chat",
    "Accept",
    "Decline",
    "Party invite",
    "Guild invite",
    "Guild",
    "Guild",
    "Not in a guild",
    "Create",
    "Leave guild",
    "Name (no spaces, ≤12)",
    "Members",
    "B Inv",
    "Z Talent",
    "C Craft",
    "V Assemble",
    "Petal Inventory",
    "Talents",
    "Crafting",
    "Assemble",
    "Primary, secondary and bag petals (including coins & craft mats). (B)",
    "Invest points in Vitality or Spin. First rank at level 1. (Z)",
    "Combine 5 identical owned petals into a higher rarity. (C)",
    "Assemble petals from bag ingredients. Result goes to bag. (V)",
    "Gamble x5",
    "Exchange (EV)",
    "Selected: {}",
    "{}% · EV≈{}",
    "Guaranteed: {} → 1",
    "Not enough petals",
    "Craft failed… petals lost",
    "Success! Got {}",
    "Exchanged for {}",
    "Drop slot throws petals on the ground; T unequips; craft uses bag stacks",
    "That name is in use or reserved for an account",
    "Invert attack (RMB / Shift)",
    "Invert defend (LMB / Space)",
    "Touch controls (joystick + A/B)",
    "Fullscreen",
    "Play time",
    "Level",
    "Messages sent",
    "Kill messages",
    "Craft messages",
    "Best score",
};

static char const *const ZH[I18n::kCount] = {
    "连接中...",
    "这朵可爱的小花叫做...",
    "出生",
    "（或按回车出生）",
    "开源 florr.io PVP 复刻",
    "怎么玩",
    "用鼠标移动",
    "左键攻击",
    "右键防御",
    "用 WASD 或方向键移动",
    "空格攻击",
    "Shift 防御",
    "将以 {} 级重生",
    "，携带：",
    "设置",
    "花瓣",
    "生物",
    "更新日志",
    "键盘移动",
    "移动辅助",
    "调试信息",
    "作者：灰烬 (aqiangei) · ashflor",
    "基于 gardn / bismuth (trigonal-bacon) · 素材：M28 等",
    "中文界面",
    "统计",
    "分数：{}",
    "等级：{}",
    "花瓣图鉴",
    "怪物图鉴",
    "排行榜",
    "继续",
    "你被击败于",
    "神秘存在",
    "（或按回车继续）",
    "生命：",
    "伤害：",
    "毒素：",
    "经验：",
    "护甲：",
    "伤害反弹：",
    "治疗：",
    "花朵生命：",
    "体伤：",
    "毒素抗性：",
    "旋转速度：",
    "内容：",
    "召唤：",
    "额外视野：",
    "攻击距离：",
    "额外伤害：",
    "额外装填：",
    "移动速度：",
    "小地图",
    "向右移动 →",
    "你在此区域等级过高",
    "你将无法攻击",
    "丢弃",
    "{} 级花朵",
    "额外花瓣槽位解锁等级 {}",
    "未命名",
    "将鼠标移到此处可禁用移动",
    "也可用 [Q] / [E] 切换副手",
    "无法显示调试信息",
    "逻辑: {:.1f}/{:.1f}/{:.1f} ms (最小/平均/最大)",
    "帧: {:.1f}/{:.1f}/{:.1f} ms (最小/平均/最大) - {:.1f} fps",
    "连接断开，代码 {}",
    "连接断开，代码 {}（\"{}\"）",
    "更早的更新日志不可用",
    "菜单",
    "继续游戏",
    "按 ESC 关闭",
    "账号",
    "密码",
    "登录",
    "注册",
    "退出账号",
    "登录成功",
    "账号需为 3-16 位字母、数字或下划线",
    "密码需为 4-32 个字符",
    "账号已存在",
    "账号不存在",
    "密码错误",
    "该账号已在线",
    "已退出账号",
    "尚未连接到服务器",
    "已登录：{}  {}级",
    "游客模式（注册账号可继承当前进度）",
    "点击输入",
    "确定返回首页？",
    "确定",
    "取消",
    "邀请附近玩家",
    "离开队伍",
    "队伍",
    "未组队",
    "队友可站在尸体旁 3 秒复活你",
    "复活中... {}%",
    "附近",
    "组队",
    "公会",
    "发送组队请求",
    "邀请入公会",
    "私聊",
    "接受",
    "拒绝",
    "组队邀请",
    "公会邀请",
    "公会",
    "公会",
    "未加入公会",
    "创建",
    "离开公会",
    "名称（无空格，≤12字）",
    "成员",
    "B 背包",
    "Z 天赋",
    "C 合成",
    "V 组装",
    "花瓣背包",
    "天赋",
    "合成",
    "组装",
    "主手、副手与背包花瓣（含钱币与合成原料）。（B）",
    "天赋点可投入体魄或转速，1 级即可加首阶。（Z）",
    "五个相同已有花瓣合成更高一级。（C）",
    "用背包花瓣组装成品，结果进背包。（V）",
    "概率合成×5",
    "期望直兑",
    "已选：{}",
    "成功率 {}% · 期望消耗≈{}",
    "直兑：{} 个换 1 个",
    "花瓣不足",
    "合成失败，花瓣已消耗",
    "成功！获得 {}",
    "直兑获得 {}",
    "丢弃槽扔到地上可捡；T 卸回背包；合成用背包堆叠",
    "该昵称已被使用或是注册账号名",
    "反转攻击（右键 / Shift）",
    "反转防御（左键 / 空格）",
    "触控操作（摇杆 + 攻防键）",
    "全屏",
    "游戏时长",
    "等级",
    "发送消息数",
    "击杀消息",
    "合成消息",
    "最高分",
};

char const *I18n::tr(Str id) {
    if (id >= kCount) return "";
    return use_chinese ? ZH[id] : EN[id];
}

char const *I18n::petal_name(PetalID::T id) {
    if (id >= PetalID::kNumPetals) return "";
    return use_chinese ? PETAL_NAME_ZH[id] : PETAL_DATA[id].name;
}

char const *I18n::petal_desc(PetalID::T id) {
    if (id >= PetalID::kNumPetals) return "";
    return use_chinese ? PETAL_DESC_ZH[id] : PETAL_DATA[id].description;
}

char const *I18n::mob_name(MobID::T id) {
    if (id >= MobID::kNumMobs) return "";
    return use_chinese ? MOB_NAME_ZH[id] : MOB_DATA[id].name;
}

char const *I18n::mob_desc(MobID::T id) {
    if (id >= MobID::kNumMobs) return "";
    return use_chinese ? MOB_DESC_ZH[id] : MOB_DATA[id].description;
}

char const *I18n::rarity_name(uint8_t rarity) {
    if (rarity >= RarityID::kNumRarities) return "";
    return use_chinese ? RARITY_ZH[rarity] : RARITY_NAMES[rarity];
}

char const *I18n::map_name(uint8_t map_id) {
    if (map_id >= MapID::kNumMaps) map_id = MapID::kGarden;
    return use_chinese ? ZONE_ZH[map_id] : MAP_EN[map_id];
}

char const *I18n::zone_name(uint32_t zone_index) {
    uint8_t mid = Game::simulation.arena_info.map_id;
    if (mid >= MapID::kNumMaps) mid = MapID::kGarden;
    MapZones const &zones = map_zones(mid);
    if (zone_index >= zones.size()) return "";
    // Single-zone maps: show the world name on the minimap
    if (zones.size() == 1) return map_name(mid);
    if (use_chinese && mid == MapID::kRoyalCapital
        && zone_index < sizeof(ROYAL_ZONE_ZH) / sizeof(ROYAL_ZONE_ZH[0]))
        return ROYAL_ZONE_ZH[zone_index];
    // Ring templates share biome label — prefer Chinese map name for first rings
    if (use_chinese && zone_index < 4 && mid != MapID::kRoyalCapital)
        return map_name(mid);
    return zones[zone_index].name;
}

std::string I18n::format_respawn_level(uint32_t level) {
    return use_chinese
        ? std::format("将以 {} 级重生", level)
        : std::format("You will respawn at level {}", level);
}

std::string I18n::format_score_line(std::string const &score) {
    return use_chinese
        ? std::format("分数：{}", score)
        : std::format("Score: {}", score);
}

std::string I18n::format_level_line(uint32_t level) {
    return use_chinese
        ? std::format("等级：{}", level)
        : std::format("Level: {}", level);
}

std::string I18n::format_lvl_flower(uint32_t level) {
    return use_chinese
        ? std::format("{} 级花朵", level)
        : std::format("Lvl {} Flower", level);
}

std::string I18n::format_extra_slot(uint32_t level) {
    return use_chinese
        ? std::format("额外花瓣槽位解锁等级 {}", level)
        : std::format("Extra petal slot at level {}", level);
}

std::string I18n::format_disconnected(uint32_t code, char const *reason) {
    if (reason == nullptr || reason[0] == 0) {
        return use_chinese
            ? std::format("连接断开，代码 {}", code)
            : std::format("Disconnected with code {}", code);
    }
    return use_chinese
        ? std::format("连接断开，代码 {}（\"{}\"）", code, reason)
        : std::format("Disconnected with code {} (\"{}\")", code, reason);
}

std::string I18n::format_logged_in(std::string const &user, uint32_t level) {
    return use_chinese
        ? std::format("已登录：{}  {}级", user, level)
        : std::format("Logged in: {}  Lv.{}", user, level);
}

std::string I18n::format_revive_progress(uint8_t pct) {
    return use_chinese
        ? std::format("复活中... {}%", pct)
        : std::format("Reviving... {}%", pct);
}
