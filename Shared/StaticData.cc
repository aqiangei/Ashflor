#include <Shared/StaticData.hh>

#include <cmath>

uint32_t const TPS = 20;

// Tuned for TPS=20 (terminal ≈ accel/friction). Calibrate via screen-width crossing time.
float const PLAYER_ACCELERATION = 5.15f;
float const DEFAULT_FRICTION = 0.32f;

float const SUMMON_RETREAT_RADIUS = 600.0f;
float const DIGGER_SPAWN_CHANCE = 0.25f;
// Wild mobs return home when beyond this multiple of detection_radius from zone center
float const WILD_LEASH_MULT = 1.75f;

float const BASE_FLOWER_RADIUS = 25.0f;
float const BASE_PETAL_ROTATION_SPEED = 4.35f;
float const BASE_FOV = 0.9f;
float const BASE_HEALTH = 100.0f;
float const BASE_BODY_DAMAGE = 25.0f;

// Orbit extras beyond flower/moon radius (toward florr-like 40/100/15 feel)
float const PETAL_ORBIT_IDLE = 40.0f;
float const PETAL_ORBIT_ATTACK = 100.0f;
float const PETAL_ORBIT_DEFEND = 15.0f;
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
        .damage = 6.0,
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
        .health = 40.0,
        .damage = 26.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
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
        .damage = 6.0,
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
        .health = 18.0,
        .damage = 12.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
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
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 8.0,
        .damage = 5.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            // Was 1.65 (+65%) — felt like a rocket. +30% still cuts map travel
            // time meaningfully without making chase / PvE feel broken.
            .move_speed_factor = 1.30f,
            .equipment = EquipmentFlags::kAccelerator
        }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 8.0,
        .damage = 5.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
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
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 30, .damage = 18, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUnusual, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 90, .damage = 54, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kRare, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 270, .damage = 162, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kEpic, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 810, .damage = 486, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kLegendary, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 2430, .damage = 1458, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kMythic, .attributes = {} },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 9, .damage = 210, .radius = 7, .reload = 6.5, .count = 1, .rarity = RarityID::kRare, .attributes = {} },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 27, .damage = 630, .radius = 7, .reload = 6.5, .count = 1, .rarity = RarityID::kEpic, .attributes = {} },
    // Mythic: ×3 from Legendary (appended kStingerLegendary); triple-clump identity
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 243, .damage = 5670, .radius = 7, .reload = 6.0, .count = 3, .rarity = RarityID::kMythic, .attributes = { .clump_radius = 10 } },
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
    // ---- Jungle petals (v1 data-driven) ----
    {
        .name = "Leaf Blade",
        .description = "A sharp green leaf that cuts quickly",
        .health = 7.5, .damage = 9.0, .radius = 12.0, .reload = 1.875, .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin toxic needle from the canopy",
        .health = 5.5, .damage = 7.0, .radius = 10.0, .reload = 1.625, .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 22.0, .damage = 5.0, .radius = 17.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 8.5, .damage = 15.0, .radius = 16.0, .reload = 3.25, .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 7.0, .damage = 17.0, .radius = 13.0, .reload = 3.75, .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 9.0, .damage = 8.0, .radius = 12.0, .reload = 2.5, .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 9, .poison_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 15.0, .damage = 11.0, .radius = 15.0, .reload = 3.5, .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Mantis Claw",
        .description = "A scythe-like claw for heavy strikes",
        .health = 9.0, .damage = 21.0, .radius = 17.0, .reload = 4.75, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened toxic heart that armor-plates you",
        .health = 18.0, .damage = 6.0, .radius = 15.0, .reload = 3.75, .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 8 }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of damaging spores",
        .health = 8.0, .damage = 14.0, .radius = 16.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 11.0, .damage = 7.0, .radius = 14.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Heartwood",
        .description = "The living core of the ancient treant",
        .health = 20.0, .damage = 18.0, .radius = 18.0, .reload = 5.0, .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    // Stinger Legendary fills Unusual→…→Mythic ×3 ladder (was Epic→Mythic ×9 skip)
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 81.0, .damage = 1890.0, .radius = 7.0, .reload = 6.25, .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    // Fast (UI 轻 / PetalID::kLight) Common→Mythic ×3
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 15, .damage = 24, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kUnusual, .attributes = {} },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 45, .damage = 72, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kRare, .attributes = {} },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 135, .damage = 216, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kEpic, .attributes = {} },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 405, .damage = 648, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kLegendary, .attributes = {} },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 1215, .damage = 1944, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kMythic, .attributes = {} },
    // Heavy Common→Mythic ×3
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 60, .damage = 60, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kUnusual, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 180, .damage = 180, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kRare, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 540, .damage = 540, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kEpic, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 1620, .damage = 1620, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kLegendary, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 4860, .damage = 4860, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kMythic, .attributes = {} },
    // Twin Unusual→Mythic ×3 (base is Unusual, count=2)
    { .name = "Twin", .description = "Why stop at one? Why not TWO?!", .health = 15, .damage = 24, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kRare, .attributes = {} },
    { .name = "Twin", .description = "Why stop at one? Why not TWO?!", .health = 45, .damage = 72, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kEpic, .attributes = {} },
    { .name = "Twin", .description = "Why stop at one? Why not TWO?!", .health = 135, .damage = 216, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kLegendary, .attributes = {} },
    { .name = "Twin", .description = "Why stop at one? Why not TWO?!", .health = 405, .damage = 648, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kMythic, .attributes = {} },
    {
        .name = "Coin",
        .description = "A small gold coin stamped with a petal. Spend it at the shop.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        // Jungle-exclusive. Thrown on the ground it roots into a healing vine.
        // Aura radius grows with rarity (see Monstera ladder at end of PETAL_DATA).
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 14.0,
        .damage = 5.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 8.0,
            .aura_radius = 240.0,
            .aura_duration = 18.0,
            .defend_only = 1,
            .icon_angle = -0.35f
        }
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden leaf scrap used for crafting.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "A dry garden twig.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "A tiny bee barb.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Rare jelly from elite bees.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Heartwood of an ancient garden spirit.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert sand grain.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Poisoned desert barb.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick cactus sap.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard desert shell plate.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Eye of a living sandstorm.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Core of the sandworm monarch.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "A dense ocean water drop.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Ocean salt crystals.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "A colorful seashell.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Pearl from the deep.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Core of the abyssal tide.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Sharp polar ice crystal.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft snow fluff.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost-tempered iron shard.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of an ice beast.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Heart of endless winter.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Eye of a blizzard wisp.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living jungle vine.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber jungle resin.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "A swollen venom sac.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Jungle spore dust.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Chip of ancient heartwood.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupted spore core.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin ant shell.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Ant antenna scrap.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Concentrated formic acid.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Cluster of ant eggs.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Pheromone of the ant queen.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Heart of the ant hive.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Volcanic fire crystal.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Yellow sulfur chunk.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Still-warm lava rock.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Sharp obsidian shard.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten leyline core.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Ember of a phoenix.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Refined capital steel.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "A precision gear.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Fragment of holy light.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Royal capital badge.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Core of fallen sovereignty.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Wax royal seal.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Bee Sting",
        .description = "A quick sting that inflicts a brief poison.",
        .health = 18.0,
        .damage = 10.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 3.0, .time = 2.0 } }
    },
    {
        .name = "Healing Bloom",
        .description = "Gently restores health while equipped.",
        .health = 24.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 2.0, .defend_only = 1 }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle sand spike that hits hard.",
        .health = 14.0,
        .damage = 23.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Craft Cactus",
        .description = "A sturdy cactus that reflects a little damage.",
        .health = 55.0,
        .damage = 5.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .damage_reflection = 0.08, .extra_health = 12 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Briefly slows enemies on contact.",
        .health = 29.0,
        .damage = 8.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Water Bubble",
        .description = "A soft bubble that absorbs hits and splashes on contact.",
        .health = 40.0,
        .damage = 3.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 8, .splash_damage = 8.0, .splash_radius = 55.0, .defend_only = 1 }
    },
    {
        .name = "Starfish",
        .description = "Every third hit hits harder.",
        .health = 34.0,
        .damage = 17.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.75f }
    },
    {
        .name = "Tide Pearl",
        .description = "Extends orbit reach and pulses a water ring when attacking.",
        .health = 30.0,
        .damage = 12.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 2.5,
            .extra_range = 18,
            .radial_pulse_damage = 14.0,
            .radial_pulse_radius = 90.0
        }
    },
    {
        .name = "Ice Spike",
        .description = "A sharp ice spike that briefly slows its target.",
        .health = 20.0,
        .damage = 19.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 0.35 }
    },
    {
        .name = "Snowball",
        .description = "Small snowballs that quickly return when broken.",
        .health = 18.0,
        .damage = 10.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ice Shield",
        .description = "A reinforced ice shield that reduces incoming damage.",
        .health = 62.0,
        .damage = 2.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 10, .defend_only = 1 }
    },
    {
        .name = "Thorn Vine",
        .description = "A thorny vine that snares and slows enemies on contact.",
        .health = 29.0,
        .damage = 7.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 1.1, .snare_seconds = 0.8 }
    },
    {
        .name = "Rotwood",
        .description = "Rotting wood that inflicts lingering poison.",
        .health = 37.0,
        .damage = 8.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 4.0, .time = 5.0 } }
    },
    {
        .name = "Ant Fang",
        .description = "A sharp ant fang that bites harder into poisoned foes.",
        .health = 19.0,
        .damage = 21.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Acid Flask",
        .description = "Applies a weak but long-lasting poison.",
        .health = 21.0,
        .damage = 4.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 2.0, .time = 6.0 } }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant to fight for you.",
        .health = 48.0,
        .damage = 0.0,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Fireball",
        .description = "A volatile fireball that burns on contact.",
        .health = 16.0,
        .damage = 25.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "A heavy molten core that splashes molten damage on contact.",
        .health = 59.0,
        .damage = 24.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 10, .splash_damage = 12.0, .splash_radius = 70.0 }
    },
    {
        .name = "Obsidian Blade",
        .description = "A fragile obsidian blade that cuts through armor.",
        .health = 18.0,
        .damage = 35.0,
        .radius = 12.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Steel Sword",
        .description = "A reliable steel sword with solid damage and durability.",
        .health = 38.0,
        .damage = 25.0,
        .radius = 12.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 6 }
    },
    {
        .name = "Holy Orb",
        .description = "Releases a burst of healing when activated.",
        .health = 30.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 20.0, .defend_only = 1 }
    },
    {
        .name = "Mech Gear",
        .description = "A mechanical gear that increases petal rotation speed.",
        .health = 40.0,
        .damage = 19.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.18 }
    },
    {
        .name = "Thorn Palm",
        .description = "A thorny palm that strikes back when damaged.",
        .health = 78.0,
        .damage = 20.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.18 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Creates a temporary healing aura.",
        .health = 42.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 3.0,
            .aura_radius = 95.0,
            .aura_duration = 4.0,
            .defend_only = 1
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 30.0,
        .damage = 22.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.0,
            .defend_only = 1,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 86.0,
        .damage = 10.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 1.5, .extra_health = 22 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty barrier that reduces incoming damage.",
        .health = 72.0,
        .damage = 5.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 8, .defend_only = 1 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost starfish provide sustained contact pressure.",
        .health = 28.0,
        .damage = 15.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 48.0,
        .damage = 16.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 3.0,
            .slow_inflict_seconds = 0.45,
            .extra_range = 25,
            .radial_pulse_damage = 18.0,
            .radial_pulse_radius = 110.0
        }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 39.0,
        .damage = 29.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.12f, .bonus_vs_slowed = 0.25f }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you.",
        .health = 56.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 3.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 46.0,
        .damage = 9.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 6.0, .time = 5.0 } }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed.",
        .health = 58.0,
        .damage = 9.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .move_speed_factor = 1.08f }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with strong poison resistance.",
        .health = 82.0,
        .damage = 6.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 5, .poison_armor = 18 }
    },
    {
        .name = "Forged Holy Sword",
        .description = "A forged holy sword that strengthens attacks and cuts armor.",
        .health = 48.0,
        .damage = 32.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.08f, .armor_penetration = 0.25f }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 44.0,
        .damage = 23.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 0.25, .extra_range = 8 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 112.0,
        .damage = 3.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.28, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 46.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 36.0, .defend_only = 1 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 44.0,
        .damage = 6.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 4.5, .on_hit_heal = 2.0 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 22.0,
        .damage = 13.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .move_speed_factor = 1.11f }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance.",
        .health = 49.0,
        .damage = 14.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 20, .poison_armor = 6 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 55.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 3.0, .extra_health = 15, .defend_only = 1 }
    },
    {
        .name = "Molten Sand Blade",
        .description = "A fragile molten blade that delivers heavy strikes.",
        .health = 26.0,
        .damage = 42.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 65.0,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 3.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 70.0,
        .damage = 30.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .damage_reflection = 0.10,
            .burn_damage = { .damage = 5.0, .time = 3.0 }
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 24.0,
        .damage = 20.0,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 66.0,
        .damage = 28.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 12, .slow_inflict_seconds = 0.3 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 21.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 16.0, .defend_only = 1 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 53.0,
        .damage = 17.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 4, .extra_range = 30 }
    },
    {
        .name = "Frost Crown",
        .description = "A frost crown that increases durability and poison resistance.",
        .health = 79.0,
        .damage = 7.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 32, .poison_armor = 8 }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes.",
        .health = 36.0,
        .damage = 33.0,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_damage_factor = 1.16f,
            .execute_threshold = 0.25f,
            .execute_factor = 1.6f
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 34.0,
        .damage = 17.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_rotation_speed = 0.14,
            .poison_damage = { .damage = 3.0, .time = 2.0 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 45.0,
        .damage = 12.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .constant_heal = 1.5,
            .poison_damage = { .damage = 3.0, .time = 3.0 }
        }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 110.0,
        .damage = 8.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 16, .extra_health = 18, .taunt_radius = 200.0 }
    },
    {
        .name = "Heart of Nature",
        .description = "Greatly increases durability and steadily restores health.",
        .health = 95.0,
        .damage = 0.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 7.0, .extra_health = 35, .defend_only = 1 }
    },
    {
        .name = "Abyssal Tide",
        .description = "Extends orbit and pulses an abyssal ring when attacking.",
        .health = 90.0,
        .damage = 32.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 3.5,
            .slow_inflict_seconds = 0.85,
            .extra_range = 42,
            .radial_pulse_damage = 28.0,
            .radial_pulse_radius = 140.0
        }
    },
    {
        .name = "Infernal Core",
        .description = "A massive infernal core that burns foes on contact.",
        .health = 112.0,
        .damage = 57.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_health = 25,
            .burn_damage = { .damage = 8.0, .time = 4.0 }
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Roots into a time field that slows nearby foes.",
        .health = 80.0,
        .damage = 14.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.5,
            .slow_inflict_seconds = 2.0,
            .aura_radius = 120.0,
            .aura_duration = 6.0,
            .aura_slow_seconds = 1.2,
            .defend_only = 1
        }
    },
    {
        .name = "Ant King",
        .description = "Hatches two soldier ants to fight for you.",
        .health = 115.0,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 20.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 3.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "World Tree Seed",
        .description = "Creates a large healing aura for a limited time.",
        .health = 85.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 7.0,
            .aura_radius = 155.0,
            .aura_duration = 8.0,
            .defend_only = 1
        }
    },
    {
        .name = "Skyblade",
        .description = "A divine sword that delivers exceptionally powerful strikes.",
        .health = 68.0,
        .damage = 63.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.20f }
    },
    {
        .name = "Chaos Prism",
        .description = "Inflicts poison and briefly slows enemies on contact.",
        .health = 83.0,
        .damage = 31.0,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .poison_damage = { .damage = 7.0, .time = 3.0 },
            .slow_inflict_seconds = 0.75
        }
    },
    // ---- Unique / Super / Ultra ladders (×2 from Mythic each step) ----
    // Strong Unique Basic (appended after meme kUniqueBasic so craft prefers this id).
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 4860, .damage = 2916, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 9720, .damage = 5832, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 19440, .damage = 11664, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUltra, .attributes = {} },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 486, .damage = 11340, .radius = 7, .reload = 6.0, .count = 3, .rarity = RarityID::kUnique, .attributes = { .clump_radius = 10 } },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 972, .damage = 22680, .radius = 7, .reload = 6.0, .count = 3, .rarity = RarityID::kSuper, .attributes = { .clump_radius = 10 } },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 1944, .damage = 45360, .radius = 7, .reload = 6.0, .count = 3, .rarity = RarityID::kUltra, .attributes = { .clump_radius = 10 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 810, .damage = 810, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kUnique, .attributes = { .secondary_reload = 1.0, .burst_heal = 2268, .defend_only = 1 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 1620, .damage = 1620, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kSuper, .attributes = { .secondary_reload = 1.0, .burst_heal = 4536, .defend_only = 1 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 3240, .damage = 3240, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kUltra, .attributes = { .secondary_reload = 1.0, .burst_heal = 9072, .defend_only = 1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 1620, .damage = 1296, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kUnique, .attributes = { .constant_heal = 6, .icon_angle = -1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 3240, .damage = 2592, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kSuper, .attributes = { .constant_heal = 7, .icon_angle = -1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 6480, .damage = 5184, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kUltra, .attributes = { .constant_heal = 8, .icon_angle = -1 } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 810, .damage = 810, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kUnique, .attributes = { .poison_damage = { .damage = 1620.0, .time = 6.0 } } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 1620, .damage = 1620, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kSuper, .attributes = { .poison_damage = { .damage = 3240.0, .time = 6.0 } } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 3240, .damage = 3240, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kUltra, .attributes = { .poison_damage = { .damage = 6480.0, .time = 6.0 } } },
    { .name = "Wing", .description = "It comes and goes", .health = 810, .damage = 810, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUnique, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "It comes and goes", .health = 1620, .damage = 1620, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kSuper, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "It comes and goes", .health = 3240, .damage = 3240, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUltra, .attributes = { .icon_angle = 1 } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 648, .damage = 2160, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kUnique, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 1296, .damage = 4320, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kSuper, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 2592, .damage = 8640, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kUltra, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kUnique, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kSuper, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kUltra, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 270, .damage = 378, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kUnique, .attributes = { .extra_rotation_speed = 1.75 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 540, .damage = 756, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kSuper, .attributes = { .extra_rotation_speed = 2.0 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 1080, .damage = 1512, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kUltra, .attributes = { .extra_rotation_speed = 2.25 } },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 5400, .damage = 540, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 10800, .damage = 1080, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 21600, .damage = 2160, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kUltra, .attributes = {} },
    { .name = "Web", .description = "It's really sticky", .health = 540, .damage = 270, .radius = 10, .reload = 3.0, .count = 3, .rarity = RarityID::kUnique, .attributes = { .clump_radius = 10, .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Web", .description = "It's really sticky", .health = 1080, .damage = 540, .radius = 10, .reload = 3.0, .count = 3, .rarity = RarityID::kSuper, .attributes = { .clump_radius = 10, .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Web", .description = "It's really sticky", .health = 2160, .damage = 1080, .radius = 10, .reload = 3.0, .count = 3, .rarity = RarityID::kUltra, .attributes = { .clump_radius = 10, .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 2430, .damage = 3888, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 4860, .damage = 7776, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 9720, .damage = 15552, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kUltra, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 9720, .damage = 9720, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 19440, .damage = 19440, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 38880, .damage = 38880, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kUltra, .attributes = {} },
    { .name = "Twin", .description = "Why stop at one? Why not TWO?!", .health = 810, .damage = 1296, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Twin", .description = "Why stop at one? Why not TWO?!", .health = 1620, .damage = 2592, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Twin", .description = "Why stop at one? Why not TWO?!", .health = 3240, .damage = 5184, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kUltra, .attributes = {} },
    // ---- Ant Hell exclusive (anthell.txt) ----
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 22.0, .damage = 21.0, .radius = 8.0, .reload = 0.85, .count = 1,
        .rarity = RarityID::kUnusual, .attributes = {}
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 70.0, .damage = 8.0, .radius = 12.0, .reload = 3.5, .count = 1,
        .rarity = RarityID::kUnusual, .attributes = { .armor = 4 }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that burns on contact.",
        .health = 18.0, .damage = 8.0, .radius = 9.0, .reload = 3.5, .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 4.0, .time = 3.5 } }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 35.0, .damage = 4.0, .radius = 11.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 2.5, .defend_only = 1 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 28.0, .damage = 10.0, .radius = 10.0, .reload = 2.0, .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.55, .move_speed_factor = 1.06f }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone that resists knockback.",
        .health = 95.0, .damage = 12.0, .radius = 13.0, .reload = 5.0, .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 80.0, .armor = 3 }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible armor that punches through shells.",
        .health = 40.0, .damage = 28.0, .radius = 10.0, .reload = 1.1, .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_damage_factor = 1.15f }
    },
    {
        .name = "Hive Disc",
        .description = "A thick hive disc that spins as durable defense.",
        .health = 140.0, .damage = 14.0, .radius = 13.0, .reload = 4.5, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 6, .extra_rotation_speed = 0.25 }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 90.0, .damage = 10.0, .radius = 12.0, .reload = 5.5, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 4, .poison_damage = { .damage = 5.0, .time = 4.0 } }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 180.0, .damage = 12.0, .radius = 13.0, .reload = 5.0, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 10, .extra_health = 25 }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 55.0, .damage = 0.0, .radius = 12.0, .reload = 16.0, .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 5.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and stirs the swarm.",
        .health = 120.0, .damage = 16.0, .radius = 14.0, .reload = 14.0, .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 1.0,
            .constant_heal = 4.0,
            .burst_heal = 12.0,
            .extra_rotation_speed = 0.4
        }
    },
    // ---- Monstera ladder (aura_radius rises each rarity; combat ×3 / ×2 like other ladders) ----
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 42.0, .damage = 15.0, .radius = 15.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.5, .aura_heal = 11.0, .aura_radius = 290.0,
            .aura_duration = 20.0, .defend_only = 1, .icon_angle = -0.35f
        }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 126.0, .damage = 45.0, .radius = 15.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.5, .aura_heal = 15.0, .aura_radius = 350.0,
            .aura_duration = 22.0, .defend_only = 1, .icon_angle = -0.35f
        }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 378.0, .damage = 135.0, .radius = 15.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.5, .aura_heal = 21.0, .aura_radius = 420.0,
            .aura_duration = 24.0, .defend_only = 1, .icon_angle = -0.35f
        }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 756.0, .damage = 270.0, .radius = 15.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5, .aura_heal = 28.0, .aura_radius = 500.0,
            .aura_duration = 26.0, .defend_only = 1, .icon_angle = -0.35f
        }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 1512.0, .damage = 540.0, .radius = 15.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.5, .aura_heal = 36.0, .aura_radius = 590.0,
            .aura_duration = 28.0, .defend_only = 1, .icon_angle = -0.35f
        }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 3024.0, .damage = 1080.0, .radius = 15.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5, .aura_heal = 45.0, .aura_radius = 700.0,
            .aura_duration = 30.0, .defend_only = 1, .icon_angle = -0.35f
        }
    },
    // ---- Honey / Meteor (art stubs; AFK/control gameplay wiring later) ----
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 220.0,
        .damage = 5.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 900.0,
        .damage = 1.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    },
    // ---- Honey / Meteor rarity ladders ----
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 24.0,
        .damage = 0.6,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 73.0,
        .damage = 1.7,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 660.0,
        .damage = 15.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 1980.0,
        .damage = 45.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 5940.0,
        .damage = 135.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 11880.0,
        .damage = 270.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 23760.0,
        .damage = 540.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 47520.0,
        .damage = 1080.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mass = 0.35f, .defend_only = 1 }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 40.0,
        .damage = 0.2,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 120.0,
        .damage = 0.4,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 300.0,
        .damage = 0.6,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 600.0,
        .damage = 0.8,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 2700.0,
        .damage = 2.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 5400.0,
        .damage = 4.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 10800.0,
        .damage = 8.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    },
    {
        .name = "Meteor",
        .description = "A hollow meteor crater ring. Extremely durable ground obstacle with a standable center.",
        .health = 21600.0,
        .damage = 16.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mass = 0.55f, .defend_only = 1, .rotation_style = PetalAttributes::kNoRot }
    }
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
            PetalID::kLight, PetalID::kLeaf, PetalID::kChitinNeedle, PetalID::kShellPlate,
            PetalID::kMatAntShell, PetalID::kMatAntenna, PetalID::kMatFormicAcid
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
            PetalID::kTwin, PetalID::kIris, PetalID::kWing, PetalID::kChitinNeedle,
            PetalID::kMatAntShell, PetalID::kMatFormicAcid
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
            PetalID::kLight, PetalID::kStinger, PetalID::kTwin, PetalID::kWing,
            PetalID::kMatBeeBarb, PetalID::kMatPollenDust, PetalID::kHoney
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
            PetalID::kDandelion, PetalID::kMissile, PetalID::kWing, PetalID::kBubble, PetalID::kAntennae,
            PetalID::kMatBeeBarb, PetalID::kMatRoyalJelly, PetalID::kHoney
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
            PetalID::kStinger, PetalID::kYucca, PetalID::kCactus,
            PetalID::kMatCactusSap, PetalID::kMatVenomBarb
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
            PetalID::kHeavy, PetalID::kLight, PetalID::kRock, PetalID::kMeteor
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
            PetalID::kHeavy, PetalID::kRock, PetalID::kHeaviest, PetalID::kMoon, PetalID::kMeteor
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
            PetalID::kLight, PetalID::kTwin, PetalID::kLeaf, PetalID::kPeas,
            PetalID::kMatLeaf, PetalID::kMatTwig
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
            PetalID::kSand, PetalID::kFaster, PetalID::kMatSand, PetalID::kMatSandstormEye
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
            PetalID::kColonyHeart, PetalID::kQueensEmblem, PetalID::kAntEgg,
            PetalID::kMatAntRoe, PetalID::kMatQueenPheromone, PetalID::kMatHiveHeart
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
        .drops = { PetalID::kIronGuard, PetalID::kMatSteel, PetalID::kMatGear },
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
        .radius = {34.2},
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
        .radius = {35.4},
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
        .radius = {24.4},
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
        .radius = {28.1},
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
        .radius = {34.2},
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
        .radius = {47.6},
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
        .radius = {39.0},
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
        .radius = {30.5},
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
        .radius = {54.9},
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
        .radius = {112.2},
        .xp = 200,
        .drops = { PetalID::kCrownFragment, PetalID::kKingsOath, PetalID::kRoyalSpear },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Jungle mobs ----
    {
        .name = "Leaf Beetle",
        .description = "A beetle wearing leaf armor.",
        .rarity = RarityID::kCommon,
        .health = {35.0}, .damage = 10.0, .radius = {41.5}, .xp = 4,
        .drops = { PetalID::kLeafBlade, PetalID::kEmeraldShell, PetalID::kMonstera },
        .attributes = { .aggro_radius = 550 }
    },
    {
        .name = "Jungle Mite",
        .description = "Small, fast, and everywhere in the undergrowth.",
        .rarity = RarityID::kCommon,
        .health = {14.0}, .damage = 8.0, .radius = {34.2}, .xp = 2,
        .drops = { PetalID::kJungleNeedle, PetalID::kPoisonSpore },
        .attributes = { .aggro_radius = 480 }
    },
    {
        .name = "Vine Bug",
        .description = "A plant bug wrapped in living vines.",
        .rarity = RarityID::kUnusual,
        .health = {70.0}, .damage = 12.0, .radius = {34.2}, .xp = 8,
        .drops = { PetalID::kVineWhip, PetalID::kThornRing, PetalID::kMonstera },
        .attributes = { .aggro_radius = 600 }
    },
    {
        .name = "Jungle Spider",
        .description = "A canopy spider with a venomous bite.",
        .rarity = RarityID::kUnusual,
        .health = {45.0}, .damage = 14.0, .radius = {31.7}, .xp = 7,
        .drops = { PetalID::kSpiderFang, PetalID::kPoisonSpore },
        .attributes = { .aggro_radius = 650 }
    },
    {
        .name = "Emerald Mantis",
        .description = "An emerald predator with scythe limbs.",
        .rarity = RarityID::kRare,
        .health = {90.0}, .damage = 22.0, .radius = {45.1}, .xp = 16,
        .drops = { PetalID::kMantisClaw, PetalID::kLeafBlade },
        .attributes = { .aggro_radius = 700 }
    },
    {
        .name = "Poison Toad",
        .description = "A bulky toad dripping yellow toxin.",
        .rarity = RarityID::kRare,
        .health = {110.0}, .damage = 12.0, .radius = {41.5}, .xp = 14,
        .drops = { PetalID::kPoisonSpore, PetalID::kToxicHeart },
        .attributes = { .aggro_radius = 580, .poison_damage = { .damage = 5.0, .time = 3.0 } }
    },
    {
        .name = "Spore Mushroom",
        .description = "A slow mushroom that sheds spores as it walks.",
        .rarity = RarityID::kUnusual,
        .health = {55.0}, .damage = 9.0, .radius = {30.5}, .xp = 6,
        .drops = { PetalID::kPoisonSpore, PetalID::kSporeCloud },
        .attributes = { .aggro_radius = 500 }
    },
    {
        .name = "Carnivorous Plant",
        .description = "Looks like foliage until it opens its jaws.",
        .rarity = RarityID::kRare,
        .health = {120.0}, .damage = 18.0, .radius = {46.4}, .xp = 18,
        .drops = { PetalID::kVineWhip, PetalID::kToxicHeart, PetalID::kMonstera },
        .attributes = { .aggro_radius = 620 }
    },
    {
        .name = "River Crab",
        .description = "A heavy blue-green crab of the jungle riverbanks.",
        .rarity = RarityID::kUnusual,
        .health = {85.0}, .damage = 15.0, .radius = {37.8}, .xp = 10,
        .drops = { PetalID::kEmeraldShell, PetalID::kThornRing },
        .attributes = { .aggro_radius = 560 }
    },
    {
        .name = "Stone Guardian",
        .description = "A vine-wrapped stone sentinel of the ruins.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 16.0, .radius = {56.1}, .xp = 28,
        .drops = { PetalID::kAncientSeed, PetalID::kEmeraldShell },
        .attributes = { .aggro_radius = 680 }
    },
    {
        .name = "Jungle Wasp",
        .description = "A striped wasp that never leaves the canopy alone.",
        .rarity = RarityID::kUnusual,
        .health = {32.0}, .damage = 20.0, .radius = {29.3}, .xp = 8,
        .drops = { PetalID::kJungleNeedle, PetalID::kSpiderFang },
        .attributes = { .aggro_radius = 720 }
    },
    {
        .name = "Ancient Treant",
        .description = "The living heartwood of the jungle.",
        .rarity = RarityID::kUnique,
        .health = {2800.0}, .damage = 26.0, .radius = {115.9}, .xp = 220,
        .drops = { PetalID::kAncientSeed, PetalID::kHeartwood, PetalID::kVineWhip, PetalID::kMonstera },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Ocean mobs ----
    {
        .name = "Reef Stone",
        .description = "A pale reef boulder resting on the sandy seabed.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {26.8}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Sea Fry",
        .description = "A timid little fish that only fights back.",
        .rarity = RarityID::kCommon,
        .health = {14.0}, .damage = 10.0, .radius = {31.7}, .xp = 2,
        .drops = { PetalID::kBubble, PetalID::kLeaf },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Moon Jelly",
        .description = "A translucent drifter that drifts toward warm light.",
        .rarity = RarityID::kCommon,
        .health = {24.0}, .damage = 12.0, .radius = {41.5}, .xp = 4,
        .drops = { PetalID::kBubble, PetalID::kMatWaterDrop, PetalID::kMatSeaSalt },
        .attributes = { .aggro_radius = 220 }
    },
    {
        .name = "Coral Star",
        .description = "A slow orange starfish that guards its coral.",
        .rarity = RarityID::kCommon,
        .health = {35.0}, .damage = 15.0, .radius = {34.2}, .xp = 5,
        .drops = { PetalID::kRose, PetalID::kMatSeashell, PetalID::kMatCoral },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Reef Crab",
        .description = "A broad-shelled crab with two heavy claws.",
        .rarity = RarityID::kCommon,
        .health = {40.0}, .damage = 18.0, .radius = {39.0}, .xp = 6,
        .drops = { PetalID::kPincer, PetalID::kHeavy, PetalID::kRock },
        .attributes = { .aggro_radius = 260 }
    },
    {
        .name = "Spine Puffer",
        .description = "A round pufferfish that bristles when angered.",
        .rarity = RarityID::kUnusual,
        .health = {75.0}, .damage = 18.0, .radius = {42.7}, .xp = 10,
        .drops = { PetalID::kHeavy, PetalID::kPoisonPeas, PetalID::kBubble },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Sea Leech",
        .description = "A six-segment drifter that coils through the kelp.",
        .rarity = RarityID::kUnusual,
        .health = {65.0}, .damage = 16.0, .radius = {36.6}, .xp = 9,
        .drops = { PetalID::kSalt, PetalID::kPoisonPeas, PetalID::kAntennae },
        .attributes = { .aggro_radius = 280, .segments = 6 }
    },
    {
        .name = "Moray Eel",
        .description = "A long eel that lunges from its crevice.",
        .rarity = RarityID::kUnusual,
        .health = {85.0}, .damage = 20.0, .radius = {46.4}, .xp = 12,
        .drops = { PetalID::kStinger, PetalID::kCutter, PetalID::kFaster },
        .attributes = { .aggro_radius = 350 }
    },
    {
        .name = "Ink Cuttle",
        .description = "A shadow-cuttlefish that waits, then strikes.",
        .rarity = RarityID::kRare,
        .health = {110.0}, .damage = 19.0, .radius = {51.2}, .xp = 22,
        .drops = { PetalID::kWeb, PetalID::kObserver, PetalID::kMoon, PetalID::kSalt },
        .attributes = { .aggro_radius = 320 }
    },
    {
        .name = "Abyss Shark",
        .description = "A deep-water hunter that owns the trench.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 22.0, .radius = {61.0}, .xp = 35,
        .drops = { PetalID::kCutter, PetalID::kHeaviest, PetalID::kThirdEye, PetalID::kMissile },
        .attributes = { .aggro_radius = 450 }
    },
    {
        .name = "Abyss Maw",
        .description = "A deep-sea maw that swallows whole the lightless dark.",
        .rarity = RarityID::kUnique,
        .health = {2700.0}, .damage = 28.0, .radius = {115.9}, .xp = 220,
        .drops = { PetalID::kLotus, PetalID::kYinYang, PetalID::kHeaviest, PetalID::kMoon },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Snow mobs ----
    {
        .name = "Frost Stone",
        .description = "A blue ice boulder under a thin blanket of snow.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy, PetalID::kSalt },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Snow Mite",
        .description = "A tiny burrower that hides under the snow.",
        .rarity = RarityID::kCommon,
        .health = {12.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = { PetalID::kLight, PetalID::kMatIceCrystal, PetalID::kMatSnowFluff },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Snow Hare",
        .description = "An alert white hare that bolts at the first sound.",
        .rarity = RarityID::kCommon,
        .health = {18.0}, .damage = 10.0, .radius = {36.6}, .xp = 3,
        .drops = { PetalID::kFaster, PetalID::kMatSnowFluff, PetalID::kMatIceCrystal },
        .attributes = { .aggro_radius = 260 }
    },
    {
        .name = "Ice Spider",
        .description = "A pale spider that lays freezing webs on the ground.",
        .rarity = RarityID::kCommon,
        .health = {32.0}, .damage = 14.0, .radius = {30.5}, .xp = 5,
        .drops = { PetalID::kWeb, PetalID::kTriweb, PetalID::kSalt },
        .attributes = { .aggro_radius = 280 }
    },
    {
        .name = "Frost Hornet",
        .description = "A cold-blue wasp that charges when it spots you.",
        .rarity = RarityID::kCommon,
        .health = {38.0}, .damage = 18.0, .radius = {34.2}, .xp = 6,
        .drops = { PetalID::kStinger, PetalID::kWing, PetalID::kTringer },
        .attributes = { .aggro_radius = 350 }
    },
    {
        .name = "Rime Centipede",
        .description = "A ten-segment rime crawler that hunts in the pines.",
        .rarity = RarityID::kUnusual,
        .health = {65.0}, .damage = 16.0, .radius = {39.0}, .xp = 10,
        .drops = { PetalID::kTwin, PetalID::kTriplet, PetalID::kAntennae },
        .attributes = { .aggro_radius = 320, .segments = 10 }
    },
    {
        .name = "Blizzard Wisp",
        .description = "A swirling snow cloud that drifts without purpose.",
        .rarity = RarityID::kUnusual,
        .health = {48.0}, .damage = 15.0, .radius = {36.6}, .xp = 8,
        .drops = { PetalID::kFaster, PetalID::kBubble, PetalID::kMoon },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Glacier Beast",
        .description = "A heavy polar beast with two stubby horns.",
        .rarity = RarityID::kUnusual,
        .health = {85.0}, .damage = 20.0, .radius = {48.8}, .xp = 12,
        .drops = { PetalID::kHeavy, PetalID::kHeaviest, PetalID::kBone },
        .attributes = { .aggro_radius = 360 }
    },
    {
        .name = "Frost Yeti",
        .description = "A long-armed giant that stalks the frost rifts.",
        .rarity = RarityID::kRare,
        .health = {140.0}, .damage = 20.0, .radius = {54.9}, .xp = 25,
        .drops = { PetalID::kBone, PetalID::kHeaviest, PetalID::kYggdrasil, PetalID::kRice },
        .attributes = { .aggro_radius = 420 }
    },
    {
        .name = "Ice Golem",
        .description = "A slow colossus of stacked ice and frozen stone.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 22.0, .radius = {61.0}, .xp = 35,
        .drops = { PetalID::kRock, PetalID::kHeaviest, PetalID::kThirdEye, PetalID::kLotus },
        .attributes = { .aggro_radius = 380 }
    },
    {
        .name = "Winter Sovereign",
        .description = "An ancient snow sovereign crowned with frozen spikes.",
        .rarity = RarityID::kUnique,
        .health = {2700.0}, .damage = 28.0, .radius = {115.9}, .xp = 220,
        .drops = { PetalID::kLotus, PetalID::kYinYang, PetalID::kHeaviest, PetalID::kYggdrasil },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Volcano mobs ----
    {
        .name = "Ember Stone",
        .description = "A cooling volcanic rock with dull red cracks.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy, PetalID::kSalt },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Ash Mite",
        .description = "A grey mite that skitters through the volcanic ash.",
        .rarity = RarityID::kCommon,
        .health = {14.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = { PetalID::kLight, PetalID::kPollen, PetalID::kLeaf },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Cinder Moth",
        .description = "An ash-grey moth that flutters away from trouble.",
        .rarity = RarityID::kCommon,
        .health = {20.0}, .damage = 10.0, .radius = {23.2}, .xp = 3,
        .drops = { PetalID::kWing, PetalID::kFaster, PetalID::kDandelion },
        .attributes = { .aggro_radius = 250 }
    },
    {
        .name = "Lava Beetle",
        .description = "A beetle with glowing seams running across its shell.",
        .rarity = RarityID::kCommon,
        .health = {36.0}, .damage = 16.0, .radius = {35.4}, .xp = 5,
        .drops = { PetalID::kBeetleEgg, PetalID::kHeavy, PetalID::kRock },
        .attributes = { .aggro_radius = 280 }
    },
    {
        .name = "Ember Wasp",
        .description = "A slender orange wasp that charges after a short windup.",
        .rarity = RarityID::kCommon,
        .health = {38.0}, .damage = 20.0, .radius = {34.2}, .xp = 6,
        .drops = { PetalID::kStinger, PetalID::kMatFireCrystal, PetalID::kMatPhoenixEmber },
        .attributes = { .aggro_radius = 350 }
    },
    {
        .name = "Magma Crab",
        .description = "A hard-shelled crab with magma glowing beneath its plate.",
        .rarity = RarityID::kUnusual,
        .health = {85.0}, .damage = 18.0, .radius = {46.4}, .xp = 12,
        .drops = { PetalID::kPincer, PetalID::kMatLavaRock, PetalID::kMatSulfur },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Fire Spider",
        .description = "A dark red spider whose bite leaves a burning sting.",
        .rarity = RarityID::kUnusual,
        .health = {55.0}, .damage = 16.0, .radius = {37.8}, .xp = 9,
        .drops = { PetalID::kWeb, PetalID::kPoisonPeas, PetalID::kTriweb },
        .attributes = { .aggro_radius = 300, .poison_damage = { .damage = 6.0, .time = 2.0 } }
    },
    {
        .name = "Magma Centipede",
        .description = "A six-segment crawler that glows like cooling lava.",
        .rarity = RarityID::kUnusual,
        .health = {75.0}, .damage = 20.0, .radius = {43.9}, .xp = 11,
        .drops = { PetalID::kTwin, PetalID::kTriplet, PetalID::kAntennae },
        .attributes = { .aggro_radius = 320, .segments = 6 }
    },
    {
        .name = "Obsidian Golem",
        .description = "A towering obsidian figure veined with orange light.",
        .rarity = RarityID::kRare,
        .health = {150.0}, .damage = 20.0, .radius = {56.1}, .xp = 28,
        .drops = { PetalID::kRock, PetalID::kHeaviest, PetalID::kBone, PetalID::kCutter },
        .attributes = { .aggro_radius = 380 }
    },
    {
        .name = "Lava Colossus",
        .description = "A hulking lava beast that barrels straight ahead.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 22.0, .radius = {61.0}, .xp = 35,
        .drops = { PetalID::kHeaviest, PetalID::kThirdEye, PetalID::kCutter, PetalID::kGoldenLeaf },
        .attributes = { .aggro_radius = 440 }
    },
    {
        .name = "Molten Overlord",
        .description = "The molten heart of the volcano, crowned with bent rock horns.",
        .rarity = RarityID::kUnique,
        .health = {2750.0}, .damage = 28.0, .radius = {115.9}, .xp = 220,
        .drops = { PetalID::kYinYang, PetalID::kHeaviest, PetalID::kThirdEye, PetalID::kGoldenLeaf },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Desert mobs (new) ----
    // Cactus / Sandstorm / Scorpion / DesertCentipede above are reused unchanged.
    {
        .name = "Dune Stone",
        .description = "A flat wind-worn sandstone with a single eroded groove.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {28.1}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Sand Mite",
        .description = "A tiny mite that crawls across the hot sand.",
        .rarity = RarityID::kCommon,
        .health = {12.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = { PetalID::kLight, PetalID::kSand, PetalID::kAntennae },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Dune Beetle",
        .description = "A broad-backed desert beetle with a sand-tan shell.",
        .rarity = RarityID::kCommon,
        .health = {38.0}, .damage = 16.0, .radius = {35.4}, .xp = 5,
        .drops = { PetalID::kBeetleEgg, PetalID::kHeavy, PetalID::kSand },
        .attributes = { .aggro_radius = 280 }
    },
    {
        .name = "Dune Spider",
        .description = "A sand-coloured spider that spreads sticky nets.",
        .rarity = RarityID::kUnusual,
        .health = {60.0}, .damage = 16.0, .radius = {37.8}, .xp = 9,
        .drops = { PetalID::kWeb, PetalID::kTriweb, PetalID::kSand },
        .attributes = { .aggro_radius = 300 }
    },
    {
        .name = "Dune Hornet",
        .description = "A slender sand-gold wasp that charges on sight.",
        .rarity = RarityID::kUnusual,
        .health = {55.0}, .damage = 18.0, .radius = {36.6}, .xp = 9,
        .drops = { PetalID::kStinger, PetalID::kWing, PetalID::kTringer },
        .attributes = { .aggro_radius = 360 }
    },
    {
        .name = "Dune Titan",
        .description = "A colossal desert carapace that moves with glacial weight.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 22.0, .radius = {61.0}, .xp = 35,
        .drops = { PetalID::kHeaviest, PetalID::kBeetleEgg, PetalID::kBone, PetalID::kThirdEye },
        .attributes = { .aggro_radius = 400 }
    },
    {
        .name = "Sandworm Monarch",
        .description = "The monarch of the sandworm nest, jaws of hooked stone.",
        .rarity = RarityID::kUnique,
        .health = {2750.0}, .damage = 28.0, .radius = {115.9}, .xp = 220,
        .drops = { PetalID::kBone, PetalID::kHeaviest, PetalID::kYinYang, PetalID::kYucca },
        .attributes = { .aggro_radius = 900, .segments = 10 }
    },
    // ---- Spawn-point fillers (biomes that lacked a stationary + passive pair) ----
    {
        .name = "Jungle Stone",
        .description = "A moss-caked stone half swallowed by the jungle floor.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {28.1}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Jungle Crawler",
        .description = "A tiny crawler that keeps to the leaf litter.",
        .rarity = RarityID::kCommon,
        .health = {14.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = { PetalID::kLeaf, PetalID::kLight, PetalID::kPollen },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Royal Boulder",
        .description = "A squared-off block of palace masonry, long since cracked.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {28.1}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Royal Pebble",
        .description = "A small chipped stone that keeps rolling out of the way.",
        .rarity = RarityID::kCommon,
        .health = {14.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = { PetalID::kLight, PetalID::kPollen, PetalID::kLeaf },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Flower Guardian",
        .description = "The ancient beetle that nests in the flower heart.",
        .rarity = RarityID::kUnique,
        .health = {2600.0}, .damage = 26.0, .radius = {112.2}, .xp = 200,
        .drops = { PetalID::kIris, PetalID::kWing, PetalID::kBeetleEgg, PetalID::kThirdEye, PetalID::kTriplet },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Ant Hell mobs (new) — classic ants above kept unchanged ----
    {
        .name = "Acid Ant",
        .description = "Keeps its distance and spits acid from a pale green sac.",
        .rarity = RarityID::kUnusual,
        .health = {35.0}, .damage = 12.0, .radius = {41.5}, .xp = 6,
        .drops = {
            PetalID::kAcidDroplet, PetalID::kAcidFlask, PetalID::kMatFormicAcid, PetalID::kMatAntShell
        },
        .attributes = { .aggro_radius = 360, .poison_damage = { .damage = 5.0, .time = 2.0 } }
    },
    {
        .name = "Scout Ant",
        .description = "A slim, fast scout that dashes in short bursts.",
        .rarity = RarityID::kCommon,
        .health = {18.0}, .damage = 10.0, .radius = {34.2}, .xp = 3,
        .drops = {
            PetalID::kChitinNeedle, PetalID::kFaster, PetalID::kMatAntenna, PetalID::kMatAntShell
        },
        .attributes = { .aggro_radius = 420 }
    },
    {
        .name = "Armored Ant",
        .description = "A wide, slow ant under thick polygonal plating.",
        .rarity = RarityID::kUnusual,
        .health = {90.0}, .damage = 14.0, .radius = {51.2}, .xp = 10,
        .drops = {
            PetalID::kIronMandible, PetalID::kRoyalCarapace, PetalID::kMatAntShell, PetalID::kAntFang
        },
        .attributes = { .aggro_radius = 280 }
    },
    {
        .name = "Brood Carrier",
        .description = "Carries a pale egg on its back and releases baby ants.",
        .rarity = RarityID::kUnusual,
        .health = {45.0}, .damage = 10.0, .radius = {46.4}, .xp = 8,
        .drops = {
            PetalID::kBroodPearl, PetalID::kSoldierEgg, PetalID::kMatAntRoe, PetalID::kMatAntShell
        },
        .attributes = { .aggro_radius = 300 }
    },
    {
        .name = "Winged Ant",
        .description = "A small ant with wide translucent wings.",
        .rarity = RarityID::kUnusual,
        .health = {30.0}, .damage = 12.0, .radius = {39.0}, .xp = 6,
        .drops = {
            PetalID::kPheromoneStar, PetalID::kWing, PetalID::kMatAntenna, PetalID::kMatAntShell
        },
        .attributes = { .aggro_radius = 380 }
    },
    {
        .name = "Tunnel Guard",
        .description = "A flat, heavy ant that barrels down nest corridors.",
        .rarity = RarityID::kRare,
        .health = {110.0}, .damage = 18.0, .radius = {53.7}, .xp = 16,
        .drops = {
            PetalID::kTunnelStone, PetalID::kHiveDisc, PetalID::kMatAntShell, PetalID::kBone
        },
        .attributes = { .aggro_radius = 340 }
    },
    {
        .name = "Spore Ant",
        .description = "A purple ant whose spore sac leaks toxins on contact.",
        .rarity = RarityID::kUnusual,
        .health = {40.0}, .damage = 10.0, .radius = {41.5}, .xp = 7,
        .drops = {
            PetalID::kVenomCocoon, PetalID::kIris, PetalID::kMatFormicAcid, PetalID::kMatAntShell
        },
        .attributes = { .aggro_radius = 300, .poison_damage = { .damage = 8.0, .time = 3.0 } }
    },
    {
        .name = "Fang Ant",
        .description = "Long triangular fangs telegraph a sudden bite.",
        .rarity = RarityID::kUnusual,
        .health = {42.0}, .damage = 20.0, .radius = {28.1}, .xp = 8,
        .drops = {
            PetalID::kIronMandible, PetalID::kAntFang, PetalID::kMatAntShell, PetalID::kMatAntenna
        },
        .attributes = { .aggro_radius = 320 }
    },
    {
        .name = "Royal Sentinel",
        .description = "Amber royal guard with a gold-plated abdomen.",
        .rarity = RarityID::kRare,
        .health = {130.0}, .damage = 18.0, .radius = {37.8}, .xp = 20,
        .drops = {
            PetalID::kQueensEmblem, PetalID::kRoyalCarapace, PetalID::kMatQueenPheromone, PetalID::kMatAntShell
        },
        .attributes = { .aggro_radius = 400 }
    },
    {
        .name = "Push Crate",
        .description = "A crate you can shove into place.",
        .rarity = RarityID::kCommon,
        .health = {500000.0},
        .damage = 0.0,
        .radius = {28.0},
        .xp = 0,
        .drops = {},
        .attributes = {}
    },
    {
        .name = "Yahaha Pad",
        .description = "A glowing mark on the ground.",
        .rarity = RarityID::kCommon,
        .health = {500000.0},
        .damage = 0.0,
        .radius = {22.0},
        .xp = 0,
        .drops = {},
        .attributes = { .stationary = 1 }
    },
    {
        .name = "Treasure Guide",
        .description = "A peculiar guide with a puzzle to share.",
        .rarity = RarityID::kUnusual,
        .health = {500000.0},
        .damage = 0.0,
        .radius = {32.0},
        .xp = 0,
        .drops = {},
        .attributes = { .stationary = 1 }
    },
    {
        .name = "Loot Chest",
        .description = "A friendly treasure chest. Walk up to open it.",
        .rarity = RarityID::kUnusual,
        .health = {500000.0},
        .damage = 0.0,
        .radius = {30.0},
        .xp = 0,
        .drops = {},
        .attributes = { .stationary = 1 }
    },
}};

// Relative rarity weights for multi-drop rolls.
// Per-mob: only rarities present in that mob's drop table are kept, then renormalized to 1.0.
static float const DROP_RARITY_WEIGHT[RarityID::kNumRarities] = {
    52.0f,  // Common
    26.0f,  // Unusual
    13.0f,  // Rare
     6.0f,  // Epic
     2.2f,  // Legendary
     0.55f, // Mythic
     0.01f, // Unique (Moon/Ygg etc. — keep very rare even before tracker gate)
     0.03f, // Super
     0.01f  // Ultra
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
    // Linear XP, uncapped. Steeper than the old 20+20×L so early levels
    // are not free from a handful of mid-tier kills.
    // L1→2: 325 · L10→11: 1000 · L50→51: 4000 · L100→101: 7750
    if (level < 1) level = 1;
    return 250u + 75u * level;
}

uint32_t score_to_level(uint32_t score) {
    uint32_t level = 1;
    for (;;) {
        uint32_t level_score = score_to_pass_level(level);
        if (score < level_score) break;
        score -= level_score;
        if (level == UINT32_MAX) break;
        ++level;
    }
    return level;
}

uint32_t level_to_score(uint32_t level) {
    uint32_t score = 0;
    for (uint32_t i = 1; i < level; ++i) {
        uint32_t add = score_to_pass_level(i);
        if (score > UINT32_MAX - add) return UINT32_MAX;
        score += add;
    }
    return score;
}

uint32_t loadout_slots_at_level(uint32_t level) {
    if (level < 1) level = 1;
    uint32_t ret = 5 + level / LEVELS_PER_EXTRA_SLOT;
    if (ret > MAX_SLOT_COUNT) return MAX_SLOT_COUNT;
    return ret;
}

float hp_at_level(uint32_t level) {
    if (level < 1) level = 1;
    uint32_t gained = level > 1 ? level - 1 : 0;
    return BASE_HEALTH + 8.0f * gained + 0.08f * gained * gained;
}

float body_damage_at_level(uint32_t level) {
    if (level < 1) level = 1;
    uint32_t gained = level > 1 ? level - 1 : 0;
    return BASE_BODY_DAMAGE + 2.5f * gained + 0.015f * gained * gained;
}
