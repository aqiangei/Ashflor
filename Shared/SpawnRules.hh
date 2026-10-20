#pragma once

#include <Shared/StaticData.hh>

#include <array>
#include <cstdint>

// 落点类别 —— 决定怪在地图上的空间偏好。
// 设计见 todo/map/map-layout-and-spawn-spec.md §4.4。
namespace SpawnClass {
    enum : uint8_t {
        kAttach = 0,  // P0 依附型：紧贴障碍（0–60 单位）
        kNearWall,    // P1 贴障型：距障碍 60–250 单位（伏击/隐蔽）
        kOpen,        // P2 开阔型：开阔地随机
        kChase,       // P3 追击型：任意可通行点
        kCorridor,    // P4 通道型：长直通道
        kAnchor,      // P5 锚点型：Zone 锚点附近
        kBoss,        // P6 Boss：地图锚点，唯一实例
        kNumClasses
    };
}

struct SpawnRule {
    uint8_t cls = SpawnClass::kChase;
    float wall_dist = 0.f;    // 与障碍的目标距离（P0 / P1）
    float min_clear = 0.f;    // 要求的最小净空半径
    float anchor_range = 0.f; // 锚点散布半径
    uint8_t explicit_rule = 0; // 1 = 人工指定，优先于属性推导
};

// 每只怪的落点规则。默认按行为属性推导，特殊怪显式覆盖。
inline std::array<SpawnRule, MobID::kNumMobs> const &spawn_rules() {
    static std::array<SpawnRule, MobID::kNumMobs> const table = []{
        std::array<SpawnRule, MobID::kNumMobs> t{};
        for (SpawnRule &r : t) r = SpawnRule{ SpawnClass::kChase, 0.f, 0.f, 0.f, 0 };

        auto set = [&](MobID::T id, uint8_t cls, float wall, float clear, float anchor) {
            t[id] = SpawnRule{ cls, wall, clear, anchor, 1 };
        };

        // P0 依附型：石头 / 巢穴 / 桩子
        set(MobID::kRock,          SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kBoulder,       SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kAntHole,       SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kDigger,        SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kReefStone,     SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kFrostStone,    SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kEmberStone,    SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kDuneStone,     SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kJungleStone,   SpawnClass::kAttach, 0.f, 0.f, 0.f);
        set(MobID::kRoyalBoulder,  SpawnClass::kAttach, 0.f, 0.f, 0.f);

        // P1 贴障型：伏击 / 拟态 / 植物
        set(MobID::kCactus,          SpawnClass::kNearWall, 120.f, 0.f, 0.f);
        set(MobID::kMimicChest,      SpawnClass::kNearWall, 60.f, 0.f, 0.f);
        set(MobID::kThornBug,        SpawnClass::kNearWall, 100.f, 0.f, 0.f);
        set(MobID::kSporeMushroom,   SpawnClass::kNearWall, 120.f, 0.f, 0.f);
        set(MobID::kCarnivorousPlant, SpawnClass::kNearWall, 100.f, 0.f, 0.f);
        set(MobID::kCoralStar,       SpawnClass::kNearWall, 120.f, 0.f, 0.f);
        set(MobID::kSpinePuffer,     SpawnClass::kNearWall, 120.f, 0.f, 0.f);
        set(MobID::kMagmaCrab,       SpawnClass::kNearWall, 150.f, 0.f, 0.f);
        set(MobID::kReefCrab,        SpawnClass::kNearWall, 120.f, 0.f, 0.f);
        set(MobID::kDarkLadybug,     SpawnClass::kNearWall, 100.f, 0.f, 0.f);
        set(MobID::kPoisonToad,      SpawnClass::kNearWall, 200.f, 0.f, 0.f);
        set(MobID::kRiverCrab,       SpawnClass::kNearWall, 200.f, 0.f, 0.f);

        // P2 开阔型：漂浮 / 游走
        set(MobID::kMoonJelly,     SpawnClass::kOpen, 0.f, 400.f, 0.f);
        set(MobID::kBlizzardWisp,  SpawnClass::kOpen, 0.f, 400.f, 0.f);
        set(MobID::kSquare,        SpawnClass::kOpen, 0.f, 0.f, 0.f);

        // P4 通道型：冲撞 / 长身怪需要直线路径
        set(MobID::kSandstorm,     SpawnClass::kCorridor, 0.f, 0.f, 0.f);
        set(MobID::kTunnelGuard,   SpawnClass::kCorridor, 0.f, 0.f, 0.f);

        // Ant Hell — near-wall / open preferences
        set(MobID::kSporeAnt,     SpawnClass::kNearWall, 120.f, 0.f, 0.f);
        set(MobID::kBroodCarrier,  SpawnClass::kNearWall, 100.f, 0.f, 0.f);
        set(MobID::kWingedAnt,    SpawnClass::kOpen, 0.f, 350.f, 0.f);
        set(MobID::kRoyalSentinel, SpawnClass::kAnchor, 0.f, 0.f, 900.f);

        // P6 Boss：固定锚点
        set(MobID::kQueenAnt,         SpawnClass::kBoss, 0.f, 0.f, 0.f);
        set(MobID::kFallenKing,       SpawnClass::kBoss, 0.f, 0.f, 0.f);
        set(MobID::kAncientTreant,     SpawnClass::kBoss, 0.f, 0.f, 0.f);
        set(MobID::kAbyssMaw,          SpawnClass::kBoss, 0.f, 0.f, 0.f);
        set(MobID::kWinterSovereign,   SpawnClass::kBoss, 0.f, 0.f, 0.f);
        set(MobID::kMoltenOverlord,    SpawnClass::kBoss, 0.f, 0.f, 0.f);
        set(MobID::kSandwormMonarch,   SpawnClass::kBoss, 0.f, 0.f, 0.f);
        set(MobID::kFlowerGuardian,    SpawnClass::kBoss, 0.f, 0.f, 0.f);
        return t;
    }();
    return table;
}

// 落点类别：人工指定的优先，否则按行为属性推导。
inline uint8_t spawn_class(MobID::T id) {
    SpawnRule const &rule = spawn_rules()[id];
    if (rule.explicit_rule) return rule.cls;
    struct MobData const &d = MOB_DATA[id];
    float radius = d.radius.get_single(0.5f);
    if (d.rarity == RarityID::kUnique && radius >= 90.f) return SpawnClass::kBoss;
    if (d.attributes.segments >= 6) return SpawnClass::kCorridor;
    if (d.attributes.stationary) return SpawnClass::kAttach;
    if (d.rarity >= RarityID::kEpic) return SpawnClass::kAnchor;
    if (d.rarity >= RarityID::kRare && radius >= 40.f) return SpawnClass::kAnchor;
    if (d.attributes.aggro_radius <= 0.f) return SpawnClass::kOpen;
    return SpawnClass::kChase;
}
