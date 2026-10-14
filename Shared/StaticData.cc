#include <Shared/StaticData.hh>

#include <cmath>

uint32_t const MAX_LEVEL = 99;
uint32_t const TPS = 20;

//these two are based on the base game's 20 TPS
float const PLAYER_ACCELERATION = 5.0f;
float const DEFAULT_FRICTION = 1.0f/3.0f;

float const PETAL_DISABLE_DELAY = 45.0f; //seconds
float const SUMMON_RETREAT_RADIUS = 600.0f;
float const DIGGER_SPAWN_CHANCE = 0.25f;

float const BASE_FLOWER_RADIUS = 25.0f;
float const BASE_PETAL_ROTATION_SPEED = 4.5f;
float const BASE_FOV = 0.9f;
float const BASE_HEALTH = 100.0f;
float const BASE_BODY_DAMAGE = 25.0f;

float const PETAL_ORBIT_IDLE = 45.0f;
float const PETAL_ORBIT_ATTACK = 110.0f;
float const PETAL_ORBIT_DEFEND = 12.0f;
float const PETAL_WING_WAVE_EXTRA = 120.0f;

std::array<struct PetalData, PetalID::kNumPetals> const PETAL_DATA = {{
    {
        .name = "None",
        .description = "How can you see this?",
        .health = 0.0, 
        .damage = 0.0,
        .radius = 0.0,
        .reload = 1.0,
        .count = 0,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 5.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 20.0,
        .damage = 20.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 3.0,
        .damage = 70.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 10.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .constant_heal = 1,
            .icon_angle = -1
        }
    },
    {
        .name = "Twin",
        .description = "Why stop at one? Why not TWO?!",
        .health = 5.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 5.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .secondary_reload = 1.0,
            .burst_heal = 14,
            .defend_only = 1
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 5.0,
        .damage = 5.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .poison_damage = {
                .damage = 10.0,
                .time = 6.0
            }
        }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 12.0,
        .damage = 40.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.85, 
            .defend_only = 1,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot 
        }
    },
    {
        .name = "Dandelion",
        .description = "Its interesting properties prevent healing effects on affected units",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot 
        }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 1.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.35,
            .defend_only = 1,
        }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 5.0,
        .damage = 7.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_rotation_speed = 1.0
        }
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 100.0,
        .damage = 10.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 15.0,
        .damage = 5.0, 
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_health = 20
        }
    },
    {
        .name = "Web",
        .description = "It's really sticky",
        .health = 10.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.5,
            .defend_only = 1,
        }
    },
    {
        .name = "Wing",
        .description = "It comes and goes",
        .health = 15.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .icon_angle = 1,
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal",
        .health = 5.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 10.0,
        .damage = 4.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Pincer",
        .description = "Stuns and poisons targets for a short duration",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .slow_inflict_seconds = 2,
            .poison_damage = {
                .damage = 5.0,
                .time = 1.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 5.0,
        .damage = 5.0,
        .radius = 7.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = { 
            .clump_radius = 10,
            .secondary_reload = 1.0,
            .burst_heal = 3.5,
            .defend_only = 1
        }
    },
    {
        .name = "Triplet",
        .description = "How about THREE?!",
        .health = 5.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 50.0,
        .damage = 1.0,
        .radius = 12.5,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Iris",
        .description = "Deals its effects quicker than traditional irises",
        .health = 10.0,
        .damage = 5.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .poison_damage = {
                .damage = 15.0,
                .time = 4.0
            }
        }
    },
    {
        .name = "Pollen",
        .description = "Asthmatics beware",
        .health = 7.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.5,
            .defend_only = 1
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 5.0,
        .damage = 2.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .poison_damage = {
                .damage = 20.0,
                .time = 0.5
            },
            .defend_only = 1,
            .split_projectile = 1,
        }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 50.0,
        .damage = 1.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Rose",
        .description = "Extremely powerful rose, almost unheard of",
        .health = 5.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .secondary_reload = 1.0,
            .burst_heal = 22,
            .defend_only = 1
        }
    },
    {
        .name = "Stick",
        .description = "Harnesses the power of the wind",
        .health = 10.0,
        .damage = 1.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { 
            .secondary_reload = 4.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
        }
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 81.0,
        .damage = 1890.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Web",
        .description = "It's really sticky",
        .health = 90.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10,
            .secondary_reload = 0.5,
            .defend_only = 1,
        }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 0.0,
        .damage = 0.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .vision_factor = 0.6,
            .equipment = EquipmentFlags::kAntennae 
        }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 15.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10,
            .extra_health = 40,
        }
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 200.0,
        .damage = 10.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 0.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_range = 75,
            .equipment = EquipmentFlags::kThirdEye
        }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 0.0,
        .damage = 0.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kMythic,
        .attributes = {
            .vision_factor = 0.25,
            .equipment = EquipmentFlags::kObserver
        }
    },
    {
        .name = "Cactus",
        .description = "Turns your flower poisonous. Enemies will take poison damage on contact",
        .health = 15.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_health = 20,
            .poison_damage = {
                .damage = 1.0,
                .time = 5.0
            }
        }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .damage_reflection = 0.25
        }
    },
    {
        .name = "Basic",
        .description = "Something incredibly rare and useless",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 10.0,
        .damage = 10.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
        }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 1000.0,
        .damage = 3.0,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .mass = 200
        }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 5.0,
        .damage = 5.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .poison_armor = 5,
            .icon_angle = 0.1
        }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 0.0,
        .damage = 0.0,
        .radius = 40.0,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .extra_body_damage = 15,
            .equipment = EquipmentFlags::kCutter
        }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 15.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 1.0,
        .damage = 1.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .defend_only = 1,
            .icon_angle = M_PI
        }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 1.0,
        .damage = 4.0,
        .radius = 13.0,
        .reload = 0.05,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Bone",
        .description = "Sturdy",
        .health = 12.0,
        .damage = 15.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .armor = 8,
            .icon_angle = 1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 10.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .constant_heal = 1.5,
            .icon_angle = -1
        }
    },
    {
        .name = "Corn", 
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 500.0,
        .damage = 2.5,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Leaf",
        .description = "Allows your petals to recharge slower but do more damage",
        .health = 10.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_damage_factor = 1.2,
            .extra_reload_factor = 1.2,
            .icon_angle = -1
        }
    },
    {
        .name = "Accelerator",
        .description = "Strap it on and your flower zooms like it's flying",
        .health = 0.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .move_speed_factor = 1.65f,
            .equipment = EquipmentFlags::kAccelerator
        }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 0.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .magnet_radius = 1800.f,
            .equipment = EquipmentFlags::kMagnet
        }
    },
    // ---- Royal Capital petals (v1 simplified) ----
    {
        .name = "Iron Guard",
        .description = "A sturdy shield petal that can intercept missiles",
        .health = 20.0,
        .damage = 5.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 5.5,
        .damage = 6.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 10.0,
        .damage = 16.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 9.0,
        .damage = 8.5,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 8.0, .time = 3.0 }
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 8.5,
        .damage = 20.0,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_range = 45.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 12.0,
        .damage = 22.0,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin (simplified aura)",
        .health = 14.0,
        .damage = 5.5,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_rotation_speed = 1.15f,
            .move_speed_factor = 1.08f
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 24.0,
        .damage = 3.5,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .armor = 10
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Marks targets with extra petal damage",
        .health = 7.0,
        .damage = 13.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.12f
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 11.0,
        .damage = 6.5,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that slows on contact (v1)",
        .health = 10.0,
        .damage = 11.5,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 1.5f,
            .poison_damage = { .damage = 4.0, .time = 2.0 }
        }
    },
    {
        .name = "King's Oath",
        .description = "A royal seal — surges when your flower is low on health",
        .health = 13.0,
        .damage = 16.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_damage_factor = 1.15f
        }
    },
    // ---- Rarity ladders (same name as base; ×3 combat feel) ----
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 30, .damage = 30, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUnusual, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 90, .damage = 90, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kRare, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 270, .damage = 270, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kEpic, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 810, .damage = 810, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kLegendary, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 2430, .damage = 2430, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kMythic, .attributes = {} },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 9, .damage = 210, .radius = 7, .reload = 6.5, .count = 1, .rarity = RarityID::kRare, .attributes = {} },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 27, .damage = 630, .radius = 7, .reload = 6.5, .count = 1, .rarity = RarityID::kEpic, .attributes = {} },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 243, .damage = 5670, .radius = 7, .reload = 6.5, .count = 3, .rarity = RarityID::kMythic, .attributes = { .clump_radius = 10 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 15, .damage = 15, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kRare, .attributes = { .secondary_reload = 1.0, .burst_heal = 42, .defend_only = 1 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 45, .damage = 45, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kEpic, .attributes = { .secondary_reload = 1.0, .burst_heal = 126, .defend_only = 1 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 135, .damage = 135, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kLegendary, .attributes = { .secondary_reload = 1.0, .burst_heal = 378, .defend_only = 1 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 405, .damage = 405, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kMythic, .attributes = { .secondary_reload = 1.0, .burst_heal = 1134, .defend_only = 1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 30, .damage = 24, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kRare, .attributes = { .constant_heal = 2, .icon_angle = -1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 90, .damage = 72, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kEpic, .attributes = { .constant_heal = 3, .icon_angle = -1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 270, .damage = 216, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kLegendary, .attributes = { .constant_heal = 4, .icon_angle = -1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 810, .damage = 648, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kMythic, .attributes = { .constant_heal = 5, .icon_angle = -1 } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 15, .damage = 15, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kRare, .attributes = { .poison_damage = { .damage = 30.0, .time = 6.0 } } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 45, .damage = 45, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kEpic, .attributes = { .poison_damage = { .damage = 90.0, .time = 6.0 } } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 135, .damage = 135, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kLegendary, .attributes = { .poison_damage = { .damage = 270.0, .time = 6.0 } } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 405, .damage = 405, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kMythic, .attributes = { .poison_damage = { .damage = 810.0, .time = 6.0 } } },
    { .name = "Wing", .description = "It comes and goes", .health = 5, .damage = 5, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUnusual, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "It comes and goes", .health = 45, .damage = 45, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kEpic, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "It comes and goes", .health = 135, .damage = 135, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kLegendary, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "It comes and goes", .health = 405, .damage = 405, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kMythic, .attributes = { .icon_angle = 1 } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 4, .damage = 13.3f, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kUnusual, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 36, .damage = 120, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kEpic, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 108, .damage = 360, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kLegendary, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 324, .damage = 1080, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kMythic, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kUnusual, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kEpic, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kLegendary, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kMythic, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 1.67f, .damage = 2.33f, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kUnusual, .attributes = { .extra_rotation_speed = 1.0 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 15, .damage = 21, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kEpic, .attributes = { .extra_rotation_speed = 1.0 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 45, .damage = 63, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kLegendary, .attributes = { .extra_rotation_speed = 1.25 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 135, .damage = 189, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kMythic, .attributes = { .extra_rotation_speed = 1.5 } },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 33.3f, .damage = 3.33f, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kUnusual, .attributes = {} },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 300, .damage = 30, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kEpic, .attributes = {} },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 900, .damage = 90, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kLegendary, .attributes = {} },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 2700, .damage = 270, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kMythic, .attributes = {} },
    { .name = "Web", .description = "It's really sticky", .health = 3.33f, .damage = 1.67f, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kUnusual, .attributes = { .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Web", .description = "It's really sticky", .health = 30, .damage = 15, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kEpic, .attributes = { .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Web", .description = "It's really sticky", .health = 270, .damage = 135, .radius = 10, .reload = 3.0, .count = 3, .rarity = RarityID::kMythic, .attributes = { .clump_radius = 10, .secondary_reload = 0.5, .defend_only = 1 } },
}};

std::array<struct MobData, MobID::kNumMobs> const MOB_DATA = {{
    {
        .name = "Baby Ant",
        .description = "Weak and defenseless, but big dreams.",
        .rarity = RarityID::kCommon,
        .health = {10.0},
        .damage = 10.0,
        .radius = {14.0},
        .xp = 1,
        .drops = {
            PetalID::kLight, PetalID::kLeaf, PetalID::kTwin, PetalID::kRice, PetalID::kTriplet
        }, 
        .attributes = {}
    },
    {
        .name = "Worker Ant",
        .description = "It's temperamental, probably from working all the time.",
        .rarity = RarityID::kCommon,
        .health = {25.0},
        .damage = 10.0,
        .radius = {14.0},
        .xp = 3,
        .drops = {
            PetalID::kLight, PetalID::kLeaf, PetalID::kTwin, PetalID::kCorn, PetalID::kBone
        }, 
        .attributes = {}
    },
    {
        .name = "Soldier Ant",
        .description = "It's got wings and it's ready to use them.",
        .rarity = RarityID::kUnusual,
        .health = {40.0},
        .damage = 10.0,
        .radius = {14.0},
        .xp = 5,
        .drops = {
            PetalID::kTwin, PetalID::kIris, PetalID::kWing, PetalID::kFaster, PetalID::kTriplet
        }, 
        .attributes = {}
    },
    {
        .name = "Bee",
        .description = "It stings. Don't touch it.",
        .rarity = RarityID::kCommon,
        .health = {28.0},
        .damage = 50.0,
        .radius = {20.0},
        .xp = 4,
        .drops = {
            PetalID::kLight, PetalID::kStinger, PetalID::kTwin, PetalID::kWing
        },
        .attributes = {}
    },
    {
        .name = "Ladybug",
        .description = "Cute and harmless.",
        .rarity = RarityID::kCommon,
        .health = {25.0},
        .damage = 10.0,
        .radius = {30.0},
        .xp = 3,
        .drops = {
            PetalID::kLight, PetalID::kRose, PetalID::kTwin, PetalID::kBubble
        },
        .attributes = {}
    },
    {
        .name = "Beetle",
        .description = "It's hungry and flowers are its favorite meal.",
        .rarity = RarityID::kUnusual,
        .health = {40.0},
        .damage = 35.0,
        .radius = {35.0},
        .xp = 10,
        .drops = {
            PetalID::kIris, PetalID::kSalt, PetalID::kWing, PetalID::kTriplet
        },
        .attributes = {}
    },
    {
        .name = "Massive Ladybug",
        .description = "Much larger, but still cute.",
        .rarity = RarityID::kEpic,
        .health = {1000.0},
        .damage = 10.0,
        .radius = {90.0},
        .xp = 400,
        .drops = {
            PetalID::kRose, PetalID::kDahlia, PetalID::kBubble, PetalID::kAzalea, PetalID::kObserver
        }, 
        .attributes = {}
    },
    {
        .name = "Massive Beetle",
        .description = "Someone overfed this one, you might be next.",
        .rarity = RarityID::kRare,
        .health = {600.0},
        .damage = 35.0,
        .radius = {75.0},
        .xp = 50,
        .drops = {
            PetalID::kIris, PetalID::kWing, PetalID::kBlueIris, PetalID::kTriplet, PetalID::kBeetleEgg, PetalID::kThirdEye
        }, 
        .attributes = { 
            .aggro_radius = 750
        }
    },
    {
        .name = "Ladybug",
        .description = "Cute and harmless... if left unprovoked.",
        .rarity = RarityID::kUnusual,
        .health = {35.0},
        .damage = 10.0,
        .radius = {30.0},
        .xp = 5,
        .drops = {
            PetalID::kDahlia, PetalID::kWing, PetalID::kYinYang, PetalID::kAzalea
        },
        .attributes = {}
    },
    {
        .name = "Hornet",
        .description = "These aren't quite as nice as the little bees.",
        .rarity = RarityID::kUnusual,
        .health = {40.0},
        .damage = 40.0,
        .radius = {40.0},
        .xp = 12,
        .drops = {
            PetalID::kDandelion, PetalID::kMissile, PetalID::kWing, PetalID::kBubble, PetalID::kAntennae
        },
        .attributes = {
            .aggro_radius = 600
        }
    },
    {
        .name = "Cactus",
        .description = "This one's prickly, don't touch it either.",
        .rarity = RarityID::kCommon,
        .health = {25.0, 50.0},
        .damage = 30.0,
        .radius = {30.0, 60.0},
        .xp = 2,
        .drops = {
            PetalID::kStinger, PetalID::kYucca, PetalID::kCactus, PetalID::kPoisonCactus, PetalID::kTricac
        },
        .attributes = {
            .stationary = 1
        }
    },
    {
        .name = "Rock",
        .description = "A rock. It doesn't do much.",
        .rarity = RarityID::kCommon,
        .health = {5.0, 15.0},
        .damage = 10.0,
        .radius = {10.0, 25.0},
        .xp = 1,
        .drops = {
            PetalID::kHeavy, PetalID::kLight, PetalID::kRock
        },
        .attributes = {
            .stationary = 1
        }
    },
    {
        .name = "Boulder",
        .description = "A bigger rock. It also doesn't do much.",
        .rarity = RarityID::kUnusual,
        .health = {40.0, 60.0},
        .damage = 10.0,
        .radius = {50.0, 75.0},
        .xp = 10,
        .drops = {
            PetalID::kHeavy, PetalID::kRock, PetalID::kHeaviest, PetalID::kMoon
        }, 
        .attributes = {
            .stationary = 1
        }
    },
    {
        .name = "Centipede",
        .description = "It's just there doing its thing.",
        .rarity = RarityID::kUnusual,
        .health = {50.0},
        .damage = 10.0,
        .radius = {35.0},
        .xp = 2,
        .drops = {
            PetalID::kLight, PetalID::kTwin, PetalID::kLeaf, PetalID::kPeas, PetalID::kTriplet, PetalID::kGoldenLeaf
        },
        .attributes = {
            .segments = 10
        }
    },
    {
        .name = "Evil Centipede",
        .description = "This one loves flowers.",
        .rarity = RarityID::kRare,
        .health = {50.0},
        .damage = 10.0,
        .radius = {35.0},
        .xp = 3,
        .drops = {
            PetalID::kIris, PetalID::kPoisonPeas, PetalID::kBlueIris
        },
        .attributes = { 
            .segments = 10, 
            .poison_damage = {
                .damage = 5.0,
                .time = 2.0
            }
        }
    },
    {
        .name = "Desert Centipede",
        .description = "It doesn't like it when you interrupt its run.",
        .rarity = RarityID::kRare,
        .health= {50.0},
        .damage = 10.0,
        .radius = {35.0},
        .xp = 4,
        .drops = {
            PetalID::kSand, PetalID::kFaster, PetalID::kSalt, PetalID::kStick
        },
        .attributes = {
            .segments = 6
        }
    },
    {
        .name = "Sandstorm",
        .description = "Quite unpredictable.",
        .rarity = RarityID::kUnusual,
        .health = {30.0, 45.0},
        .damage = 40.0,
        .radius = {32.0, 48.0},
        .xp = 5,
        .drops = {
            PetalID::kSand, PetalID::kFaster, PetalID::kStick
        },
        .attributes = {}
    },
    {
        .name = "Scorpion",
        .description = "This one stings, now with poison.",
        .rarity = RarityID::kUnusual,
        .health = {35.0},
        .damage = 10.0,
        .radius = {35.0},
        .xp = 10,
        .drops = {
            PetalID::kIris, PetalID::kPincer, PetalID::kTriplet, PetalID::kLotus
        }, 
        .attributes = {
            .poison_damage = {
                .damage = 10.0,
                .time = 1.0
            }
        }
    },
    {
        .name = "Spider",
        .description = "Spooky.",
        .rarity = RarityID::kUnusual,
        .health = {35.0},
        .damage = 10.0,
        .radius = {15.0},
        .xp = 8,
        .drops = {
            PetalID::kStinger, PetalID::kWeb, PetalID::kFaster, PetalID::kTriweb
        },
        .attributes = { 
            .poison_damage = {
                .damage = 5.0,
                .time = 3.0
            }
        }
    },
    {
        .name = "Ant Hole",
        .description = "Ants go in, and come out. Can't explain that.",
        .rarity = RarityID::kRare,
        .health = {500.0},
        .damage = 10.0,
        .radius = {45.0},
        .xp = 25,
        .drops = {
            PetalID::kIris, PetalID::kWing, PetalID::kAntEgg, PetalID::kTriplet
        },
        .attributes = {
            .stationary = 1 
        }
    },
    {
        .name = "Queen Ant",
        .description = "You must have done something really bad if she's chasing you.",
        .rarity = RarityID::kRare,
        .health = {350.0},
        .damage = 10.0,
        .radius = {25.0},
        .xp = 15,
        .drops = {
            PetalID::kTwin, PetalID::kIris, PetalID::kWing, PetalID::kAntEgg, PetalID::kTringer
        },
        .attributes = {
            .aggro_radius = 750
        }
    },
    {
        .name = "Ladybug",
        .description = "This one is shiny... I wonder what it could mean...",
        .rarity = RarityID::kEpic,
        .health = {25.0},
        .damage = 10.0,
        .radius = {30.0},
        .xp = 30,
        .drops = {
            PetalID::kDahlia, PetalID::kWing, PetalID::kBubble, PetalID::kYggdrasil
        },
        .attributes = {}
    },
    {
        .name = "Square",
        .description = "???",
        .rarity = RarityID::kUnique,
        .health = {20.0},
        .damage = 10.0,
        .radius = {40.0},
        .xp = 1,
        .drops = {
            PetalID::kSquare
        },
        .attributes = {
            .stationary = 1
        }
    },
    {
        .name = "Digger",
        .description = "Friend or foe? You'll never know...",
        .rarity = RarityID::kEpic,
        .health = {250.0},
        .damage = 25.0,
        .radius = {40.0},
        .xp = 0,
        .drops = {
            PetalID::kCutter
        },
        .attributes = {}
    },
    // ---- Royal Capital mobs ----
    {
        .name = "Wandering Guard",
        .description = "A rusted helmet still patrolling the outer walls.",
        .rarity = RarityID::kCommon,
        .health = {48.0},
        .damage = 10.0,
        .radius = {27.0},
        .xp = 6,
        .drops = { PetalID::kIronGuard, PetalID::kRoyalBanner },
        .attributes = { .aggro_radius = 550 }
    },
    {
        .name = "Stone Beetle",
        .description = "A heavy beetle plated in broken flagstones.",
        .rarity = RarityID::kUnusual,
        .health = {72.0},
        .damage = 8.0,
        .radius = {35.0},
        .xp = 10,
        .drops = { PetalID::kStoneHeart, PetalID::kIronGuard },
        .attributes = { .aggro_radius = 480 }
    },
    {
        .name = "Rust Mite",
        .description = "Tiny, fast, and covered in orange rust.",
        .rarity = RarityID::kCommon,
        .health = {20.0},
        .damage = 5.5,
        .radius = {17.0},
        .xp = 3,
        .drops = { PetalID::kRustNeedle, PetalID::kThornTrap },
        .attributes = { .aggro_radius = 600 }
    },
    {
        .name = "Mimic Chest",
        .description = "Looks like treasure. It isn't.",
        .rarity = RarityID::kRare,
        .health = {60.0},
        .damage = 14.0,
        .radius = {29.0},
        .xp = 14,
        .drops = { PetalID::kAncientCoin, PetalID::kGuardEgg },
        .attributes = { .aggro_radius = 220 }
    },
    {
        .name = "Coin Beetle",
        .description = "A golden beetle that would rather run than fight.",
        .rarity = RarityID::kUnusual,
        .health = {30.0},
        .damage = 4.0,
        .radius = {20.0},
        .xp = 8,
        .drops = { PetalID::kAncientCoin, PetalID::kKingsOath },
        .attributes = { .aggro_radius = 650 }
    },
    {
        .name = "Royal Bee",
        .description = "A striped garden bee with royal markings.",
        .rarity = RarityID::kUnusual,
        .health = {36.0},
        .damage = 11.5,
        .radius = {23.0},
        .xp = 7,
        .drops = { PetalID::kRoyalRose, PetalID::kRoyalBanner },
        .attributes = { .aggro_radius = 620 }
    },
    {
        .name = "Thorn Bug",
        .description = "A green bug bristling with thorns.",
        .rarity = RarityID::kRare,
        .health = {52.0},
        .damage = 12.0,
        .radius = {28.0},
        .xp = 11,
        .drops = { PetalID::kThornTrap, PetalID::kRoyalRose },
        .attributes = {
            .aggro_radius = 520,
            .poison_damage = { .damage = 4.0, .time = 2.0 }
        }
    },
    {
        .name = "Shield Sentinel",
        .description = "A barracks guardian with a massive forward shield.",
        .rarity = RarityID::kRare,
        .health = {80.0},
        .damage = 9.0,
        .radius = {39.0},
        .xp = 16,
        .drops = { PetalID::kIronGuard, PetalID::kStoneHeart, PetalID::kGuardEgg },
        .attributes = { .aggro_radius = 500 }
    },
    {
        .name = "Spear Sentinel",
        .description = "A red-armored spearman of the royal guard.",
        .rarity = RarityID::kRare,
        .health = {40.0},
        .damage = 16.0,
        .radius = {32.0},
        .xp = 15,
        .drops = { PetalID::kRoyalSpear, PetalID::kRoyalBanner },
        .attributes = { .aggro_radius = 580 }
    },
    {
        .name = "Phantom Orb",
        .description = "A drifting violet specter of the ruined quarter.",
        .rarity = RarityID::kEpic,
        .health = {32.0},
        .damage = 13.0,
        .radius = {25.0},
        .xp = 18,
        .drops = { PetalID::kPhantomEye, PetalID::kKingsOath },
        .attributes = { .aggro_radius = 700 }
    },
    {
        .name = "Fallen Knight",
        .description = "A corrupted knight still bound to the palace.",
        .rarity = RarityID::kLegendary,
        .health = {96.0},
        .damage = 18.0,
        .radius = {45.0},
        .xp = 35,
        .drops = { PetalID::kRoyalSpear, PetalID::kPhantomEye, PetalID::kKingsOath },
        .attributes = { .aggro_radius = 720 }
    },
    {
        // Slice C: full phase boss. Present for ID/art; not in zone spawn pools.
        .name = "Fallen King",
        .description = "The broken crown that still claims the throne.",
        .rarity = RarityID::kUnique,
        .health = {2500.0},
        .damage = 28.0,
        .radius = {92.0},
        .xp = 200,
        .drops = { PetalID::kCrownFragment, PetalID::kKingsOath, PetalID::kRoyalSpear },
        .attributes = { .aggro_radius = 900 }
    },
}};

// Relative rarity weights for guaranteed single-drop rolls.
// Per-mob: only rarities present in that mob's drop table are kept, then renormalized to 1.0.
static float const DROP_RARITY_WEIGHT[RarityID::kNumRarities] = {
    55.0f, // Common
    25.0f, // Unusual
    12.0f, // Rare
     5.5f, // Epic
     2.0f, // Legendary
     0.45f,// Mythic
     0.05f // Unique
};

std::array<StaticArray<float, MAX_DROPS_PER_MOB>, MobID::kNumMobs> const MOB_DROP_CHANCES = [](){
    std::array<StaticArray<float, MAX_DROPS_PER_MOB>, MobID::kNumMobs> ret;
    for (MobID::T id = 0; id < MobID::kNumMobs; ++id) {
        auto const &drops = MOB_DATA[id].drops;
        float rarity_w[RarityID::kNumRarities] = {};
        uint32_t rarity_n[RarityID::kNumRarities] = {};
        float total_w = 0.f;
        for (PetalID::T drop_id : drops) {
            uint8_t r = PETAL_DATA[drop_id].rarity;
            if (r >= RarityID::kNumRarities) continue;
            if (rarity_n[r] == 0) {
                rarity_w[r] = DROP_RARITY_WEIGHT[r];
                total_w += DROP_RARITY_WEIGHT[r];
            }
            rarity_n[r]++;
        }
        if (total_w <= 0.f || drops.size() == 0) {
            for (uint32_t i = 0; i < drops.size(); ++i)
                ret[id].push(drops.size() ? 1.f / drops.size() : 0.f);
            continue;
        }
        // Each petal's display chance = (rarity share / petals of that rarity); sums to 100%
        for (PetalID::T drop_id : drops) {
            uint8_t r = PETAL_DATA[drop_id].rarity;
            float share = (rarity_w[r] / total_w) / (float) rarity_n[r];
            ret[id].push(share);
        }
    }
    return ret;
}();

uint32_t score_to_pass_level(uint32_t level) {
    return (uint32_t)(pow(1.06, level - 1) * level) + 3;
}

uint32_t score_to_level(uint32_t score) {
    uint32_t level = 1;
    while (level < MAX_LEVEL) {
        uint32_t level_score = score_to_pass_level(level);
        if (score < level_score) break;
        score -= level_score;
        ++level;
    }
    return level;
}

uint32_t level_to_score(uint32_t level) {
    uint32_t score = 0;
    for (uint32_t i = 1; i < level; ++i)
        score += score_to_pass_level(i);
    return score;
}

uint32_t loadout_slots_at_level(uint32_t level) {
    if (level > MAX_LEVEL) level = MAX_LEVEL;
    uint32_t ret = 5 + level / LEVELS_PER_EXTRA_SLOT;
    if (ret > MAX_SLOT_COUNT) return MAX_SLOT_COUNT;
    return ret;
}

float hp_at_level(uint32_t level) {
    if (level > MAX_LEVEL) level = MAX_LEVEL;
    return BASE_HEALTH + level;
}