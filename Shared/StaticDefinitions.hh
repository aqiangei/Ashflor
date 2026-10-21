#pragma once

#include <Helpers/Array.hh>
#include <Helpers/Math.hh>

#include <cstdint>

// Large non-square arena — layouts generated per map_id (spirals / boxes / corridors)
inline uint32_t const ARENA_WIDTH = 28000;
inline uint32_t const ARENA_HEIGHT = 20000;
// Fixed hub spawn (map center) — buffered by difficulty-0 zone
inline float const MAP_SPAWN_X = ARENA_WIDTH * 0.5f;
inline float const MAP_SPAWN_Y = ARENA_HEIGHT * 0.5f;

inline uint32_t const MAX_SLOT_COUNT = 10;
inline uint32_t const LEVELS_PER_EXTRA_SLOT = 15;
// Soft ceiling for *future* XP gains after the zone×combat threat bug.
// Boot migration compresses only level>150 into ~101–130; see XpMigration
// and todo/xp-inflation-migration-plan.md. Safe/gray accounts are untouched.
inline uint32_t const MAX_PLAYER_LEVEL = 150;
inline uint32_t const LEADERBOARD_SIZE = 10;
// Rows kept per category on the multi-column leaderboard board
inline uint32_t const LEADERBOARD_STAT_SIZE = 10;
inline uint32_t const MINIMAP_PLAYER_CAP = 128;
inline uint32_t const MAX_PETALS_IN_CLUMP = 4;
// Matches RarityID::kNumRarities - 1 (Common..Unique). Zone difficulty == petal rarity.
inline uint32_t const MAX_DIFFICULTY = 8;
inline uint32_t const MAX_DROPS_PER_MOB = 8;
// Concept maps need many specialty Zone AABBs + large terrain pieces
inline uint32_t const MAX_ZONES_PER_MAP = 48;
// Corridor/gate/ford channel paint + pocket bridges need headroom past legacy 192.
inline uint32_t const MAX_TERRAIN_PER_MAP = 280;
inline uint32_t const TERRAIN_OUTLINE_MAX = 72;

namespace MapID {
    enum : uint8_t {
        kGarden = 0,
        kDesert,
        kOcean,
        kPolar,   // Snow / 雪原
        kJungle,
        kAntHell,
        kVolcano,
        kRoyalCapital, // 王城 — append only
        kNumMaps
    };
}

namespace TerrainKind {
    enum : uint8_t {
        kHill = 0,
        kWater,
        kChannel
    };
}

namespace TerrainShape {
    enum : uint8_t {
        kAABB = 0,   // axis-aligned box (left,top,right,bottom)
        kEllipse,    // center (left,top), radii (right,bottom)
        kCircle,     // center (left,top), radius = right
        kSpiral,     // center (left,top), outer_r = right, turns = bottom, arm half-width = param
        kRiver       // start (left,top) → end (right,bottom), half-width = param, seed = meander
    };
}

namespace TerrainFlags {
    enum : uint8_t {
        kImpassable = 1 << 0
    };
}

// Theme cloth colors (shared across maps — see MAP_TERRAIN.md).
// Ground/hill tuned for wall-vs-floor contrast (TerrainArt ThemeMaterial).
namespace MapTheme {
    inline uint32_t const GARDEN_GROUND = 0xffB9DB91;
    inline uint32_t const GARDEN_HILL = 0xff285B35;
    inline uint32_t const GARDEN_WATER = 0xff40916c;
    // Olive dirt path — warm enough to read as a road, not a pink title bar.
    inline uint32_t const GARDEN_CHANNEL = 0xffC8D6A8;
    inline uint32_t const DESERT_GROUND = 0xffF2D79B;
    inline uint32_t const DESERT_HILL = 0xff815337;
    inline uint32_t const DESERT_WATER = 0xff2a9d8f;
    inline uint32_t const DESERT_CHANNEL = 0xffC9955C;
    inline uint32_t const OCEAN_GROUND = 0xff3AA0C8;
    inline uint32_t const OCEAN_HILL = 0xff023e8a;
    inline uint32_t const OCEAN_WATER = 0xff00b4d8;
    inline uint32_t const OCEAN_CHANNEL = 0x99ff85a1;
    inline uint32_t const POLAR_GROUND = 0xffE8EEF2;
    inline uint32_t const POLAR_HILL = 0xff4A5560;
    inline uint32_t const POLAR_WATER = 0xff4cc9f0;
    inline uint32_t const POLAR_CHANNEL = 0x99caf0f8;
    inline uint32_t const JUNGLE_GROUND = 0xff52B788;
    inline uint32_t const JUNGLE_HILL = 0xff081C15;
    inline uint32_t const JUNGLE_WATER = 0xff2D6A4F;
    inline uint32_t const JUNGLE_CHANNEL = 0x9995d5b2;
    inline uint32_t const ANT_GROUND = 0xffC4A484;
    inline uint32_t const ANT_HILL = 0xff3D2314;
    inline uint32_t const ANT_WATER = 0xff582f0e;
    inline uint32_t const ANT_CHANNEL = 0x99b08968;
    inline uint32_t const VOLCANO_GROUND = 0xff4A4E6A;
    inline uint32_t const VOLCANO_HILL = 0xff0D0D14;
    inline uint32_t const VOLCANO_WATER = 0xffe76f51; // lava
    inline uint32_t const VOLCANO_CHANNEL = 0x99f4a261;
    // Royal Capital — stone plaza / palace wall / moat / carpet channel
    inline uint32_t const ROYAL_GROUND = 0xffDCC9A6;
    inline uint32_t const ROYAL_HILL = 0xff3E4355;
    inline uint32_t const ROYAL_WATER = 0xff4a6d8c;
    inline uint32_t const ROYAL_CHANNEL = 0x99c94d4c;
    // Legacy weave accents (secondary only — walls use ThemeMaterial outlines)
    inline uint32_t const WEAVE_LIGHT = 0x40ffffff;
    inline uint32_t const WEAVE_DARK = 0x30000000;
}

namespace DamageType {
    enum : uint8_t {
        kContact,
        kPoison,
        kBurn,
        kSplash, // AoE follow-up; does not re-trigger splash
        kReflect
    };
}

namespace PetalID {
    typedef uint16_t T;
    enum : T {
        kNone,
        kBasic,
        kLight,
        kHeavy,
        kStinger,
        kLeaf,
        kTwin,
        kRose,
        kIris,
        kMissile,
        kDandelion,
        kBubble,
        kFaster,
        kRock,
        kCactus,
        kWeb,
        kWing,
        kPeas,
        kSand,
        kPincer,
        kDahlia,
        kTriplet,
        kAntEgg,
        kBlueIris,
        kPollen,
        kPoisonPeas,
        kBeetleEgg,
        kAzalea,
        kStick,
        kTringer,
        kTriweb,
        kAntennae,
        kTricac,
        kHeaviest,
        kThirdEye,
        kObserver,
        kPoisonCactus,
        kSalt,
        kUniqueBasic,
        kSquare,
        kMoon,
        kLotus,
        kCutter,
        kYinYang,
        kYggdrasil,
        kRice,
        kBone,
        kYucca,
        kCorn,
        kGoldenLeaf,
        kAccelerator,
        kMagnet,
        // Royal Capital petals (append only; v1 data-driven / v2 systems later)
        kIronGuard,
        kRustNeedle,
        kAncientCoin,
        kRoyalRose,
        kRoyalSpear,
        kCrownFragment,
        kRoyalBanner,
        kStoneHeart,
        kPhantomEye,
        kGuardEgg,
        kThornTrap,
        kKingsOath,
        // Rarity ladders (same English name; append only for craft_result)
        kBasicUnusual,
        kBasicRare,
        kBasicEpic,
        kBasicLegendary,
        kBasicMythic,
        kStingerRare,
        kStingerEpic,
        kStingerMythic,
        kRoseRare,
        kRoseEpic,
        kRoseLegendary,
        kRoseMythic,
        kLeafRare,
        kLeafEpic,
        kLeafLegendary,
        kLeafMythic,
        kIrisRare,
        kIrisEpic,
        kIrisLegendary,
        kIrisMythic,
        kWingUnusual,
        kWingEpic,
        kWingLegendary,
        kWingMythic,
        kMissileUnusual,
        kMissileEpic,
        kMissileLegendary,
        kMissileMythic,
        kBubbleUnusual,
        kBubbleEpic,
        kBubbleLegendary,
        kBubbleMythic,
        kFasterUnusual,
        kFasterEpic,
        kFasterLegendary,
        kFasterMythic,
        kRockUnusual,
        kRockEpic,
        kRockLegendary,
        kRockMythic,
        kWebUnusual,
        kWebEpic,
        kWebMythic,
        // Jungle petals (append only; after rarity ladders so ladder IDs stay stable)
        kLeafBlade,
        kJungleNeedle,
        kEmeraldShell,
        kVineWhip,
        kSpiderFang,
        kPoisonSpore,
        kThornRing,
        kMantisClaw,
        kToxicHeart,
        kSporeCloud,
        kAncientSeed,
        kHeartwood,
        // Append-only ladder fill (Epic→Legendary was skipping; craft looks up by name+rarity)
        kStingerLegendary,
        // Fast/Light, Heavy, Twin — were Common/Unusual-only; craft fell back to random
        kLightUnusual,
        kLightRare,
        kLightEpic,
        kLightLegendary,
        kLightMythic,
        kHeavyUnusual,
        kHeavyRare,
        kHeavyEpic,
        kHeavyLegendary,
        kHeavyMythic,
        kTwinRare,
        kTwinEpic,
        kTwinLegendary,
        kTwinMythic,
        // Currency. Append-only. Not a combat petal and not a craft result.
        kCoin,
        // Jungle Monstera petal (append only; after kCoin so saved craft banks stay valid)
        kMonstera,
        // Craft materials + base craft products (append-only; bag petals)
        kMatLeaf,
        kMatTwig,
        kMatBeeBarb,
        kMatPollenDust,
        kMatRoyalJelly,
        kMatAncientHeart,
        kMatSand,
        kMatVenomBarb,
        kMatCactusSap,
        kMatCarapace,
        kMatSandstormEye,
        kMatSandwormCore,
        kMatWaterDrop,
        kMatSeaSalt,
        kMatSeashell,
        kMatCoral,
        kMatAbyssPearl,
        kMatTideCore,
        kMatIceCrystal,
        kMatSnowFluff,
        kMatColdIron,
        kMatIceScale,
        kMatPermafrostHeart,
        kMatBlizzardEye,
        kMatVine,
        kMatResin,
        kMatVenomSac,
        kMatSpore,
        kMatHeartwoodChip,
        kMatCorruptSpore,
        kMatAntShell,
        kMatAntenna,
        kMatFormicAcid,
        kMatAntRoe,
        kMatQueenPheromone,
        kMatHiveHeart,
        kMatFireCrystal,
        kMatSulfur,
        kMatLavaRock,
        kMatObsidian,
        kMatLeylineCore,
        kMatPhoenixEmber,
        kMatSteel,
        kMatGear,
        kMatHolyShard,
        kMatCapitalBadge,
        kMatSovereignCore,
        kMatRoyalSeal,
        // Base craft products (LeafBlade/PoisonSpore already exist)
        kBeeSting,
        kHealingBloom,
        kSandSpike,
        kCraftCactus,
        kQuicksandOrb,
        kWaterBubble,
        kStarfish,
        kTidePearl,
        kIceSpike,
        kSnowball,
        kIceShield,
        kThornVine,
        kRotwood,
        kAntFang,
        kAcidFlask,
        kSoldierEgg,
        kFireball,
        kMagmaCore,
        kObsidianBlade,
        kSteelSword,
        kHolyOrb,
        kMechGear,
        // Cross-map (32) + legendary (8) craft products
        kThornPalm,
        kPollenSandstorm,
        kSandLeafDart,
        kDesertRoot,
        kFrostBubble,
        kFrostStarfish,
        kIceTideRing,
        kDeepFrostPearl,
        kPoisonAntNest,
        kCorrodeVine,
        kColonyCore,
        kSymbioteShell,
        kForgedHolySword,
        kFlameGear,
        kObsidianKingShield,
        kHolyFlameCore,
        kAnemoneBloom,
        kWetLeaf,
        kCoralHive,
        kLifeLotus,
        kMoltenSandBlade,
        kVolcanoWorm,
        kScorchPalm,
        kGlassStorm,
        kFrostKnightSword,
        kHolySnowflake,
        kPermafrostGear,
        kFrostCrown,
        kRoyalThorn,
        kMechPoisonBee,
        kHolySpore,
        kTreantGuard,
        kHeartOfNature,
        kAbyssalTide,
        kInfernalCore,
        kChronoVortex,
        kAntKing,
        kWorldTreeSeed,
        kSkyblade,
        kChaosPrism,
        // Ultra/Super/Unique ladders (append-only; Ultra→Super→Unique craft)
        kBasicUnique,
        kBasicSuper,
        kBasicUltra,
        kStingerUnique,
        kStingerSuper,
        kStingerUltra,
        kRoseUnique,
        kRoseSuper,
        kRoseUltra,
        kLeafUnique,
        kLeafSuper,
        kLeafUltra,
        kIrisUnique,
        kIrisSuper,
        kIrisUltra,
        kWingUnique,
        kWingSuper,
        kWingUltra,
        kMissileUnique,
        kMissileSuper,
        kMissileUltra,
        kBubbleUnique,
        kBubbleSuper,
        kBubbleUltra,
        kFasterUnique,
        kFasterSuper,
        kFasterUltra,
        kRockUnique,
        kRockSuper,
        kRockUltra,
        kWebUnique,
        kWebSuper,
        kWebUltra,
        kLightUnique,
        kLightSuper,
        kLightUltra,
        kHeavyUnique,
        kHeavySuper,
        kHeavyUltra,
        kTwinUnique,
        kTwinSuper,
        kTwinUltra,
        // Ant Hell exclusive combat petals (anthell.txt — append only)
        kChitinNeedle,
        kShellPlate,
        kAcidDroplet,
        kBroodPearl,
        kPheromoneStar,
        kTunnelStone,
        kIronMandible,
        kHiveDisc,
        kVenomCocoon,
        kRoyalCarapace,
        kQueensEmblem,
        kColonyHeart,
        // Monstera rarity ladder (append only; same English name for craft_result)
        kMonsteraEpic,
        kMonsteraLegendary,
        kMonsteraMythic,
        kMonsteraUnique,
        kMonsteraSuper,
        kMonsteraUltra,
        // AFK control petals (append only; art first — gameplay wiring later)
        kHoney,
        kMeteor,
        // Honey / Meteor rarity ladders (same English name for craft_result)
        kHoneyCommon,
        kHoneyUnusual,
        kHoneyEpic,
        kHoneyLegendary,
        kHoneyMythic,
        kHoneyUnique,
        kHoneySuper,
        kHoneyUltra,
        kMeteorCommon,
        kMeteorUnusual,
        kMeteorRare,
        kMeteorEpic,
        kMeteorMythic,
        kMeteorUnique,
        kMeteorSuper,
        kMeteorUltra,
        kNumPetals
    };
};

namespace MobID {
    typedef uint8_t T;
    enum : T {
        kBabyAnt,
        kWorkerAnt,
        kSoldierAnt,
        kBee,
        kLadybug,
        kBeetle,
        kMassiveLadybug,
        kMassiveBeetle,
        kDarkLadybug,
        kHornet,
        kCactus,
        kRock,
        kBoulder,
        kCentipede,
        kEvilCentipede,
        kDesertCentipede,
        kSandstorm,
        kScorpion,
        kSpider,
        kAntHole,
        kQueenAnt,
        kShinyLadybug,
        kSquare,
        kDigger,
        // Royal Capital mobs (append only)
        kWanderingGuard,
        kStoneBeetle,
        kRustMite,
        kMimicChest,
        kCoinBeetle,
        kRoyalBee,
        kThornBug,
        kShieldSentinel,
        kSpearSentinel,
        kPhantomOrb,
        kFallenKnight,
        // Boss — Slice C full AI; Slice B may omit from zone spawns
        kFallenKing,
        // Jungle mobs (append only)
        kLeafBeetle,
        kJungleMite,
        kVineBug,
        kJungleSpider,
        kEmeraldMantis,
        kPoisonToad,
        kSporeMushroom,
        kCarnivorousPlant,
        kRiverCrab,
        kStoneGuardian,
        kJungleWasp,
        // Boss — Heart of Jungle (v1 elite_tank)
        kAncientTreant,
        // Ocean mobs (append only)
        kReefStone,
        kSeaFry,
        kMoonJelly,
        kCoralStar,
        kReefCrab,
        kSpinePuffer,
        kSeaLeech,
        kMorayEel,
        kInkCuttle,
        kAbyssShark,
        // Boss — Abyss Core
        kAbyssMaw,
        // Snow mobs (append only)
        kFrostStone,
        kSnowMite,
        kSnowHare,
        kIceSpider,
        kFrostHornet,
        kRimeCentipede,
        kBlizzardWisp,
        kGlacierBeast,
        kFrostYeti,
        kIceGolem,
        // Boss — Frost Core
        kWinterSovereign,
        // Volcano mobs (append only)
        kEmberStone,
        kAshMite,
        kCinderMoth,
        kLavaBeetle,
        kEmberWasp,
        kMagmaCrab,
        kFireSpider,
        kMagmaCentipede,
        kObsidianGolem,
        kLavaColossus,
        // Boss — Molten Heart
        kMoltenOverlord,
        // Desert mobs (append only) — Cactus/Sandstorm/Scorpion/DesertCentipede reused as-is
        kDuneStone,
        kSandMite,
        kDuneBeetle,
        kDuneSpider,
        kDuneHornet,
        kDuneTitan,
        // Boss — Sandworm Nest (10 segments)
        kSandwormMonarch,
        // Spawn-point fillers for biomes that lacked a passive pair
        kJungleStone,   // Jungle — stationary
        kJungleCrawler, // Jungle — passive
        kRoyalBoulder,  // Royal Capital — stationary
        kRoyalPebble,   // Royal Capital — passive
        // Boss — Garden Flower Heart (append only)
        kFlowerGuardian,
        // Ant Hell mobs (append only) — Baby/Worker/Soldier/AntHole/QueenAnt kept as-is
        kAcidAnt,
        kScoutAnt,
        kArmoredAnt,
        kBroodCarrier,
        kWingedAnt,
        kTunnelGuard,
        kSporeAnt,
        kFangAnt,
        kRoyalSentinel,
        // Yahaha / treasure props (append only) — push puzzles + guide NPCs
        kPushCrate,
        kYahahaPad,
        kTreasureNpc,
        kLootChest, // walk-up Yahaha chest (not Mimic)
        kNumMobs
    };
};

namespace RarityID {
    enum {
        kCommon,
        kUnusual,
        kRare,
        kEpic,
        kLegendary,
        kMythic,
        kUltra,  // 究极 — below Super
        kSuper,  // 超凡
        kUnique, // 独特 — top of ladder
        kNumRarities
    };
};

namespace ColorID {
    enum {
        kYellow,
        kGray,
        kBlue,
        kRed,
        kNumColors
    };
};

namespace AIState {
    enum {
        kIdle,
        kIdleMoving,
        kReturning,
        kBasicAggro
    };
};

namespace EntityFlags {
    enum {
        kIsDespawning,
        kNoFriendlyCollision,
        kDieOnParentDeath,
        kSpawnedFromZone,
        kNoDrops,
        kHasCulling,
        kIsCulled,
        kCPUControlled,
        kIsDetached
    };
};

namespace FaceFlags {
    enum {
        kAttacking,
        kDefending,
        kPoisoned,
        kDandelioned,
        kDeadEyes,
        kSquareEyes
    };
};

namespace EquipmentFlags {
    enum {
        kThirdEye,
        kAntennae,
        kObserver,
        kCutter,
        kAccelerator,
        kMagnet,
        kNone
    };
};

namespace InputFlags {
    enum {
        kAttacking,
        kDefending
    };
}

struct PoisonDamage {
    float damage;
    float time;
};

struct PetalAttributes {
    enum {
        kPassiveRot,
        kNoRot,
        kFollowRot
    };
    float clump_radius = 0;
    float secondary_reload = 0;
    float constant_heal = 0;
    float burst_heal = 0;
    float mass = 0.1;
    float armor = 0;
    float poison_armor = 0;
    float dandelion_inflict_seconds = 0;
    float slow_inflict_seconds = 0;
    float vision_factor = 1;
    float extra_body_damage = 0;
    float extra_rotation_speed = 0;
    float extra_range = 0;
    float extra_health = 0;
    float damage_reflection = 0;
    float extra_damage_factor = 1;
    float extra_reload_factor = 1;
    float move_speed_factor = 1; // multiplies flower acceleration when equipped
    float magnet_radius = 0; // attracts drops within this range when equipped
    // Ground "healing vine" aura: the Monstera petal, once thrown on the ground,
    // roots itself and heals every flower standing inside `aura_radius`.
    float aura_heal = 0;      // HP per second granted inside the aura
    float aura_radius = 0;    // radius of the healing area
    float aura_duration = 0;  // seconds the aura stays alive after rooting
    struct PoisonDamage poison_damage;
    struct PoisonDamage burn_damage; // contact DoT; ignores poison_armor
    float armor_penetration = 0;     // 0-1 fraction of armor ignored
    float bonus_vs_poisoned = 0;     // additive, e.g. 0.35 = +35%
    float bonus_vs_slowed = 0;
    uint8_t hit_cycle = 0;           // every N contact hits
    float hit_cycle_factor = 2.0f;
    float snare_seconds = 0;         // near-root immobilize
    float splash_damage = 0;
    float splash_radius = 0;
    float execute_threshold = 0;     // HP ratio gate
    float execute_factor = 1.5f;
    float taunt_radius = 0;          // pull mob aggro to flower
    float on_hit_heal = 0;
    float radial_pulse_damage = 0;
    float radial_pulse_radius = 0;
    float aura_slow_seconds = 0;     // slow while rooted aura active
    uint8_t defend_only = 0;
    float icon_angle = 0;
    uint8_t split_projectile = 0;
    uint8_t rotation_style = kPassiveRot;
    uint8_t spawns = MobID::kNumMobs;
    uint8_t spawn_count = 0;
    uint8_t equipment = EquipmentFlags::kNone;
};

struct PetalData {
    char const *name;
    char const *description;
    float health;
    float damage;
    float radius;
    float reload;
    uint8_t count;
    uint8_t rarity;
    struct PetalAttributes attributes;
};

struct MobAttributes {
    float aggro_radius = 500;
    uint8_t segments = 1;
    uint8_t stationary;
    struct PoisonDamage poison_damage;
};

struct MobData {
    char const *name;
    char const *description;
    uint8_t rarity;
    RangeValue health;
    float damage;
    RangeValue radius;
    uint32_t xp;
    StaticArray<PetalID::T, MAX_DROPS_PER_MOB> drops;
    struct MobAttributes attributes;
};

struct SpawnChance {
    MobID::T id;
    float chance;
};

struct ZoneDefinition {
    float left;
    float top;
    float right;
    float bottom;
    float density;
    float drop_multiplier;
    StaticArray<struct SpawnChance, MobID::kNumMobs> spawns;
    uint32_t difficulty;
    uint32_t color;
    char const *name;
    // 精英怪锚点（世界坐标）。0/0 表示使用 Zone 中心。
    float anchor_x = 0.f;
    float anchor_y = 0.f;
};

// Shape param packing — see TerrainShape. seed drives jagged edges / river meander.
// param: jagged amplitude (box/ellipse/circle), arm/river half-width (spiral/river).
struct TerrainFeature {
    uint8_t kind;
    uint8_t shape;
    uint8_t flags;
    float left;
    float top;
    float right;
    float bottom;
    uint32_t color;
    uint8_t seed = 0;
    float param = 0.f;
};
