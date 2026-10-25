#include <Shared/StaticData.hh>

#include <cmath>
#include <cstring>

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
        .health = 9.0,
        .damage = 210.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 30.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .constant_heal = 3.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 15.0,
        .damage = 24.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 15.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .secondary_reload = 0.9313,
            .burst_heal = 42.0,
            .defend_only = 1
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 15.0,
        .damage = 15.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .poison_damage = {
                .damage = 9.9999,
                .time = 2.0
            }
        }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 11.9997,
        .damage = 39.8997,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.7331, 
            .defend_only = 1,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot 
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 45.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .dandelion_inflict_seconds = 10.25,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 15.0003,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.4575, .defend_only = 1 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 45.0,
        .damage = 108.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.8875 }
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 450.0,
        .damage = 45.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 180.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { 
            .extra_health = 180.0, .damage_reflection = 0.17f }
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 15.0003,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.595, .defend_only = 1 }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 135.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal",
        .health = 45.0,
        .damage = 72.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.4313,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 9.9999,
        .damage = 9.0,
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
        .description = "Poisons and slows targets for a short duration",
        .health = 9.9999,
        .damage = 9.9999,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .slow_inflict_seconds = 2.05,
            .poison_damage = {
                .damage = 5.0004,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Dahlia",
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
        .description = "Three light petals in one slot",
        .health = 135.0,
        .damage = 216.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Egg",
        .description = "Hatches a beetle to fight for you",
        .health = 540.0,
        .damage = 27.0,
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
        .name = "Blue Iris",
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
        .description = "Scatter pollen shots when defending",
        .health = 135.0,
        .damage = 216.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.3969,
            .defend_only = 1
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 135.0,
        .damage = 216.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.3969,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Beetle Egg",
        .description = "Something interesting might pop out of this",
        .health = 540.0,
        .damage = 27.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .secondary_reload = 1.9844,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Azalea",
        .description = "An extremely powerful blossom, almost unheard of",
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
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 810.0,
        .damage = 81.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Tringer",
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
        .description = "Plant sticky webs that snare foes when defending",
        .health = 135.003,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.49, .defend_only = 1 }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 4.9977,
        .damage = 2.997,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kLegendary,
        .attributes = { .vision_factor = 0.625f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 1620.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { 
            .extra_health = 1620.0, .damage_reflection = 0.24f }
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 66.6657,
        .damage = 3.3345,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 10.0116,
        .damage = 5.0058,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_range = 84.375, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 12.0042,
        .damage = 5.0058,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kMythic,
        .attributes = { .vision_factor = 0.3187f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Cactus",
        .description = "Turns your flower poisonous. Enemies will take poison damage on contact",
        .health = 540.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .extra_health = 540.0, .damage_reflection = 0.205f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 9.9999,
        .damage = 11.9997,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .damage_reflection = 0.19f }
    },
    {
        .name = "Useless Basic",
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
        .health = 65610.0,
        .damage = 21869.8,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 3280500.0,
        .damage = 1968.3,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mass = 10.0 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 6.6663,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 27.0, .poison_armor = 162.0, .icon_angle = 0.1 }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 15.0012,
        .damage = 24.9993,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_body_damage = 324.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 19.9989,
        .damage = 19.9989,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 0.4313 }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 4050.11,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 45.0009,
        .damage = 72.0009,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 810.0,
        .damage = 1134.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 810.0, .icon_angle = 1.0 }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 9.9999,
        .damage = 5.0001,
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
        .health = 450.001,
        .damage = 45.0009,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    // Golden Leaf (黄金之叶): global petal CD cut; factors multiply across slots.
    // Rare−5% · Epic−8% · Leg−11% · Mythic−14% · Ultra−18% · Super−22% · Unique−26%.
    // World drop: Ultra only from Unique sovereigns (Molten Overlord / Ancient Treant).
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 15.0003,
        .damage = 9.9999,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 9.0, .extra_reload_factor = 0.9325f, .icon_angle = -1.0 }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 8.0001,
        .damage = 5.0001,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .move_speed_factor = 1.1738f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 9.9999,
        .damage = 5.0001,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .magnet_radius = 712.5, .equipment = EquipmentFlags::kMagnet }
    },
    // ---- Royal Capital petals (v1 simplified) ----
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 36.0,
        .damage = 5.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 1.65 }
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
        .health = 9.9999,
        .damage = 15.9999,
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
        .damage = 9.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 8.0001, .time = 3.0 }
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 9.0,
        .damage = 19.9998,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_range = 100.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 26244.0,
        .damage = 17496.2,
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
        .description = "Inspires faster petal spin while equipped",
        .health = 14.0001,
        .damage = 5.4999,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_rotation_speed = 0.315
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 24.0,
        .damage = 3.5001,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .armor = 16.5
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 9.0,
        .damage = 13.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.176f
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 315.001,
        .damage = 8.9991,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 1.9844,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 9.9999,
        .damage = 11.5002,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 1.5375,
            .poison_damage = { .damage = 3.9996, .time = 2.0 }
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 8100.0,
        .damage = 5400.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_damage_factor = 1.4500f
        }
    },
    // ---- Rarity ladders (same name as base; ×3 combat feel) ----
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 30.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 90.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 270.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 810.0,
        .damage = 810.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 2430.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 27.0,
        .damage = 630.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 81.0,
        .damage = 1890.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    // Mythic: ×3 from Legendary (appended kStingerLegendary); triple-clump identity
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 729.0,
        .damage = 17010.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 45.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { 
            .secondary_reload = 0.8625,
            .burst_heal = 126.0,
            .defend_only = 1
        }
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 135.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .secondary_reload = 0.7937,
            .burst_heal = 378.0,
            .defend_only = 1
        }
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 405.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { 
            .secondary_reload = 0.725,
            .burst_heal = 1134.0,
            .defend_only = 1
        }
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 1215.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { 
            .secondary_reload = 0.6562,
            .burst_heal = 3402.0,
            .defend_only = 1
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 90.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .constant_heal = 9.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 270.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .constant_heal = 27.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 810.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .constant_heal = 81.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 2430.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .constant_heal = 243.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 45.0,
        .damage = 45.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { 
            .poison_damage = {
                .damage = 29.9997,
                .time = 2.0
            }
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 135.0,
        .damage = 135.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .poison_damage = {
                .damage = 89.9991,
                .time = 2.0
            }
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 405.0,
        .damage = 405.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { 
            .poison_damage = {
                .damage = 269.997,
                .time = 2.0
            }
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 1215.0,
        .damage = 1215.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { 
            .poison_damage = {
                .damage = 809.992,
                .time = 2.0
            }
        }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 45.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 405.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 1215.0,
        .damage = 810.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 3645.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 3.9999,
        .damage = 13.2999,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.7916, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 35.9991,
        .damage = 119.699,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.6747, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 107.997,
        .damage = 359.097,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.6162, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 323.992,
        .damage = 1077.29,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.5578, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 5.0001,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.5038, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 45.0009,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.4113, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 135.003,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.365, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 405.008,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.3187, .defend_only = 1 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 15.0,
        .damage = 36.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_rotation_speed = 0.6688 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 135.0,
        .damage = 324.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 1.1063 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 405.0,
        .damage = 972.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 1.325 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 1215.0,
        .damage = 2916.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 1.5438 }
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 150.0,
        .damage = 15.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 1350.0,
        .damage = 135.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 4050.0,
        .damage = 405.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 12150.0,
        .damage = 1215.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 5.0001,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.6475, .defend_only = 1 }
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 45.0009,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.5425, .defend_only = 1 }
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 405.008,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.4375, .defend_only = 1 }
    },
    // ---- Jungle petals (v1 data-driven) ----
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 25.0,
        .damage = 12.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 5.5,
        .damage = 7.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 39.9999,
        .damage = 5.0001,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 8.25 }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 9.0,
        .damage = 15.0003,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 9.0,
        .damage = 17.0001,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 36.0,
        .damage = 12.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 12.0, .time = 4.0 } }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 15.0003,
        .damage = 10.9998,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 27.0,
        .damage = 27.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 18.0,
        .damage = 9.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 39.6 }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 108.0,
        .damage = 45.0009,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 5.9994, .time = 3.0 } }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 135.0,
        .damage = 45.0009,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 1.9844,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 87479.8,
        .damage = 26244.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 17677.3, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    // Stinger Legendary fills Unusual→…→Mythic ×3 ladder (was Epic→Mythic ×9 skip)
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 243.0,
        .damage = 5670.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    // Fast (UI 轻 / PetalID::kLight) Common→Mythic ×3
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 15.0,
        .damage = 24.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 45.0,
        .damage = 72.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 135.0,
        .damage = 216.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 405.0,
        .damage = 648.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 1215.0,
        .damage = 1944.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    // Heavy Common→Mythic ×3
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 60.0,
        .damage = 60.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 180.0,
        .damage = 180.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 540.0,
        .damage = 540.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 1620.0,
        .damage = 1620.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 4860.0,
        .damage = 4860.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    // Twin Unusual→Mythic ×3 (base is Unusual, count=2)
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 45.0,
        .damage = 72.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 135.0,
        .damage = 216.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 405.0,
        .damage = 648.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 1215.0,
        .damage = 1944.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
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
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 3.3336,
        .damage = 0.5553,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.6125, .aura_heal = 36.0, .aura_radius = 210.0, .aura_duration = 16.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
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
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
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
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
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
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 486.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 2268.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
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
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 486.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 2268.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
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
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
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
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 486.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 2268.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
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
        .description = "Soft fluff from snow hares. Forges Snowball.",
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
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 2268.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 486.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
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
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 2268.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 486.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
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
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 486.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 2268.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
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
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 2268.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 486.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 24.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 108.0,
        .damage = 72.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 2268.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 486.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 18.0,
        .damage = 18.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 3.0, .time = 2.0 } }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 18.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 6.0, .defend_only = 1 }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 32.0001,
        .damage = 15.9999,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 65.0001,
        .damage = 9.9999,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 36.0, .damage_reflection = 0.235f }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 21.9996,
        .damage = 8.0001,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 2.05 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 15.0003,
        .damage = 9.9999,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 72.0, .splash_damage = 72.0, .splash_radius = 495.0, .defend_only = 1 }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 39.9996,
        .damage = 15.0003,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.9031f }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 27.9999,
        .damage = 11.9997,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 2.1562,
            .extra_range = 18.0,
            .radial_pulse_damage = 14.0004,
            .radial_pulse_radius = 90.0
        }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 21.9999,
        .damage = 20.0001,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 1.2937 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 45.0,
        .damage = 11.0001,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 110.0,
        .damage = 6.0003,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 90.0, .defend_only = 1 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 54.9999,
        .damage = 12.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 0.9487, .snare_seconds = 0.69 }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 120.0,
        .damage = 8.0001,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 6.0003, .time = 2.0 } }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 30.0,
        .damage = 18.9999,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .bonus_vs_poisoned = 0.4f }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 15.9999,
        .damage = 9.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 3.0, .time = 4.0 } }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 119.999,
        .damage = 2.9997,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 1.9844,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 20.0001,
        .damage = 21.9999,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .burn_damage = { .damage = 3.9999, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 225.0,
        .damage = 83.9997,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 90.0, .burn_damage = { .damage = 6.0003, .time = 3.0 }, .splash_damage = 108.0, .splash_radius = 630.0 }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 72.0,
        .damage = 102.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor_penetration = 0.5625f }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 150.0,
        .damage = 96.0003,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 54.0 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 72.0,
        .damage = 54.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.8625, .burst_heal = 180.0, .defend_only = 1 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 240.0,
        .damage = 60.0003,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.216 }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 89.9991,
        .damage = 24.0003,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.255f }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 40.0005,
        .damage = 8.0001,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.3969, .slow_inflict_seconds = 3.5625, .aura_heal = 2.9997, .aura_radius = 94.9995, .aura_duration = 5.575, .defend_only = 1 }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 19.9989,
        .damage = 16.9992,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 1.5875,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 159.999,
        .damage = 5.0004,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 1.5012, .extra_health = 594.0 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 24.9993,
        .damage = 24.0003,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 216.0, .slow_inflict_seconds = 2.375, .defend_only = 1 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 45.0009,
        .damage = 26.001,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 50.0013,
        .damage = 18.0009,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.3813,
            .slow_inflict_seconds = 0.5344,
            .extra_range = 24.9993,
            .radial_pulse_damage = 18.0009,
            .radial_pulse_radius = 110.0007
        }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 84.9987,
        .damage = 29.9997,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.18f, .bonus_vs_slowed = 0.3625f }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 54.999,
        .damage = 8.0001,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.3813,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 40.0005,
        .damage = 23.0013,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 5.9994, .time = 5.0004 } }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 50.0013,
        .damage = 7.0011,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .move_speed_factor = 1.1813f }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 189.999,
        .damage = 5.0004,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 135.0, .poison_armor = 267.3 }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 54.999,
        .damage = 48.0006,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.14f, .armor_penetration = 0.3625f }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 59.9994,
        .damage = 34.0011,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 0.375, .extra_range = 8.0001 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 240.0,
        .damage = 8.0001,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.205f, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 70.0002,
        .damage = 24.9993,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.7937, .burst_heal = 972.0, .defend_only = 1 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 45.0009,
        .damage = 10.0008,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 5.0004, .on_hit_heal = 29.7 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 35.0001,
        .damage = 21.9996,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 2.375, .move_speed_factor = 1.2141f }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 40.0005,
        .damage = 11.9988,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 20.0, .poison_armor = 89.1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 50.0013,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 3.9987, .extra_health = 405.0, .defend_only = 1 }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 40.0005,
        .damage = 40.0005,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .burn_damage = { .damage = 162.0, .time = 3.0 } }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 80.001,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.7781,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 130.0,
        .damage = 10.0008,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .damage_reflection = 0.255f,
            .burn_damage = { .damage = 135.0, .time = 1.0 }
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 35.0001,
        .damage = 16.0002,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.3969,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 50.0013,
        .damage = 42.0012,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 2.375, .extra_health = 324.0 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 40.0005,
        .damage = 5.0004,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.7937, .constant_heal = 2.9997, .burst_heal = 432.0, .defend_only = 1 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 70.0002,
        .damage = 27.999,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 108.0, .extra_range = 29.9997 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 150.001,
        .damage = 5.0004,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_armor = 118.8, .extra_health = 864.0 }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 54.999,
        .damage = 35.9991,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_damage_factor = 1.22f,
            .execute_threshold = 0.325f,
            .execute_factor = 1.81f
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 35.0001,
        .damage = 16.0002,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_rotation_speed = 0.21,
            .poison_damage = { .damage = 2.9997, .time = 2.0007 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 29.9997,
        .damage = 3.9987,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .constant_heal = 2.0007,
            .poison_damage = { .damage = 2.9997, .time = 2.9997 }
        }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 259.999,
        .damage = 8.0001,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 432.0, .extra_health = 486.0, .taunt_radius = 225.0 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 3780.0,
        .damage = 107.997,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 197.996, .extra_health = 4680.0, .defend_only = 1 }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 1440.0,
        .damage = 810.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.2,
            .slow_inflict_seconds = 1.6,
            .extra_range = 50.0,
            .radial_pulse_damage = 494.999,
            .radial_pulse_radius = 180.0
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 2520.0,
        .damage = 855.004,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_health = 21600.0,
            .burn_damage = { .damage = 162.0, .time = 5.0 }
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 1440.0,
        .damage = 359.996,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.35,
            .slow_inflict_seconds = 3.2,
            .aura_radius = 160.0,
            .aura_duration = 12.0,
            .aura_slow_seconds = 2.2,
            .defend_only = 1
        }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 100.003,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.175,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 1800.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.35, .constant_heal = 89.9991, .aura_heal = 162.0, .aura_radius = 200.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 1080.0,
        .damage = 1260.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.5500f }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 810.0,
        .damage = 630.002,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 1.8, .poison_damage = { .damage = 251.999, .time = 3.5 } }
    },
    // ---- Ultra / Super / Unique ladders (×2 from Mythic each step) ----
    // Strong Unique Basic (appended after meme kUniqueBasic so craft prefers this id).
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 65610.0,
        .damage = 65610.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 21870.0,
        .damage = 21870.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 7290.0,
        .damage = 7290.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 19683.0,
        .damage = 459270.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 6561.0,
        .damage = 153090.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 2187.0,
        .damage = 51030.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 32805.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .secondary_reload = 0.45,
            .burst_heal = 17677.3,
            .defend_only = 1
        }
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 10935.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .secondary_reload = 0.5187,
            .burst_heal = 10206.0,
            .defend_only = 1
        }
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 3645.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .secondary_reload = 0.5875,
            .burst_heal = 5892.44,
            .defend_only = 1
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 65610.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .constant_heal = 1262.67,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 21870.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .constant_heal = 729.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 7290.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .constant_heal = 420.888,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 32805.0,
        .damage = 32805.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .poison_damage = {
                .damage = 21869.8,
                .time = 2.0
            }
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 10935.0,
        .damage = 10935.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .poison_damage = {
                .damage = 7289.93,
                .time = 2.0
            }
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 3645.0,
        .damage = 3645.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .poison_damage = {
                .damage = 2429.98,
                .time = 2.0
            }
        }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 98415.0,
        .damage = 65610.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 32805.0,
        .damage = 21870.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 10935.0,
        .damage = 7290.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 8747.78,
        .damage = 29086.9,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.3825, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 2915.93,
        .damage = 9695.63,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.4409, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 971.976,
        .damage = 3231.88,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.4994, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 10935.2,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.18, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 3645.07,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.2263, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 1215.02,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.2725, .defend_only = 1 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 32805.0,
        .damage = 78732.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 2.2 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 10935.0,
        .damage = 26244.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 1.9813 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 3645.0,
        .damage = 8748.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 1.7625 }
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 328050.0,
        .damage = 32805.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 109350.0,
        .damage = 10935.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 36450.0,
        .damage = 3645.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 10935.2,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.28, .defend_only = 1 }
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 3645.07,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.3325, .defend_only = 1 }
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 1215.02,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.385, .defend_only = 1 }
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 32805.0,
        .damage = 52488.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 10935.0,
        .damage = 17496.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 3645.0,
        .damage = 5832.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 131220.0,
        .damage = 131220.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 43740.0,
        .damage = 43740.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 14580.0,
        .damage = 14580.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 32805.0,
        .damage = 52488.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 10935.0,
        .damage = 17496.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 3645.0,
        .damage = 5832.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    // ---- Ant Hell exclusive (anthell.txt) ----
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 21.9999,
        .damage = 11.0001,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 51.9999,
        .damage = 8.0001,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 6.6 }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 18.0,
        .damage = 8.0001,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 6.9999, .time = 1.5 } }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 35.0001,
        .damage = 9.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 2.5002, .defend_only = 1 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 27.9999,
        .damage = 9.9999,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.384 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 95.0004,
        .damage = 11.9997,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 71.0, .armor = 14.85 }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 39.9996,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_damage_factor = 1.2075f }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 140.0,
        .damage = 27.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 89.1, .extra_rotation_speed = 0.375 }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 89.9991,
        .damage = 27.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 59.4, .poison_damage = { .damage = 8.9991, .time = 2.5 } }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 180.001,
        .damage = 27.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 148.5, .extra_health = 371.25 }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 674.997,
        .damage = 135.003,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 3.625,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 540.003,
        .damage = 216.003,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.725,
            .constant_heal = 4.0014,
            .burst_heal = 534.6,
            .extra_rotation_speed = 0.72
        }
    },
    // ---- Monstera ladder (aura_radius rises each rarity; combat ×3 / ×2 like other ladders) ----
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 10.0008,
        .damage = 1.6659,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.5687, .aura_heal = 108.0, .aura_radius = 245.0, .aura_duration = 18.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 30.0024,
        .damage = 4.9977,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.525, .aura_heal = 324.0, .aura_radius = 280.0, .aura_duration = 20.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 90.0072,
        .damage = 14.9931,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.4812, .aura_heal = 972.0, .aura_radius = 315.0, .aura_duration = 22.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 2430.19,
        .damage = 404.814,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.35, .aura_heal = 5050.66, .aura_radius = 420.0, .aura_duration = 28.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 810.065,
        .damage = 134.938,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.3937, .aura_heal = 2916.0, .aura_radius = 385.0, .aura_duration = 26.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 270.022,
        .damage = 44.9793,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.4375, .aura_heal = 1683.55, .aura_radius = 350.0, .aura_duration = 24.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    // ---- Honey / Meteor (art stubs; AFK/control gameplay wiring later) ----
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 4500.0,
        .damage = 9.9999,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.5625, .mass = 0.3625 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 3240.0,
        .damage = 648.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.475, .mass = 2.5, .aura_duration = 21.0 }
    },
    // ---- Honey / Meteor rarity ladders ----
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 500.0,
        .damage = 1.1111,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.65, .mass = 0.25 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 1500.0,
        .damage = 3.3333,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.6063, .mass = 0.3063 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 13500.0,
        .damage = 29.9997,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.5188, .mass = 0.4187 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 40500.0,
        .damage = 89.9991,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.475, .mass = 0.475 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 121500.0,
        .damage = 269.997,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.4313, .mass = 0.5312 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 3280500.0,
        .damage = 7289.93,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.3, .mass = 0.7 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 1093500.0,
        .damage = 2429.98,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.3438, .mass = 0.6437 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Throw it straight — durable bait mobs prefer.",
        .health = 364500.0,
        .damage = 809.992,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.3875, .mass = 0.5875 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 40.0,
        .damage = 8.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.65, .mass = 1.2, .aura_duration = 12.0 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 120.0,
        .damage = 24.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.6063, .mass = 1.5, .aura_duration = 14.25 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 360.0,
        .damage = 72.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.5625, .mass = 1.8, .aura_duration = 16.5 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 1080.0,
        .damage = 216.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.5188, .mass = 2.2, .aura_duration = 18.75 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 9720.0,
        .damage = 1944.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.4313, .mass = 2.8, .aura_duration = 23.25 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 29160.0,
        .damage = 5832.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.3875, .mass = 3.2, .aura_duration = 25.5 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 87480.0,
        .damage = 17496.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.3438, .mass = 3.6, .aura_duration = 27.75 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 262440.0,
        .damage = 52488.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.3, .mass = 4.0, .aura_duration = 30.0 }
    },
    // ---- Golden Leaf ladder (CD reduction; stacks multiplicatively) ----
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 45.0009,
        .damage = 29.9997,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 27.0, .extra_reload_factor = 0.9137f, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 135.003,
        .damage = 89.9991,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 81.0, .extra_reload_factor = 0.895f, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 405.008,
        .damage = 269.997,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 243.0, .extra_reload_factor = 0.8762f, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 1215.02,
        .damage = 809.992,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 420.888, .extra_reload_factor = 0.8575f, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 3645.07,
        .damage = 2429.98,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 729.0, .extra_reload_factor = 0.8387f, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 10935.2,
        .damage = 7289.93,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 1262.67, .extra_reload_factor = 0.82f, .icon_angle = -1.0 }
    },
    // ---- Auto-filled rarity ladders (gen_petal_ladders.py) ----
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 4320.01,
        .damage = 2430.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 2.42,
            .slow_inflict_seconds = 1.76,
            .extra_range = 55.0,
            .radial_pulse_damage = 1485.0,
            .radial_pulse_radius = 198.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 12960.0,
        .damage = 7290.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 2.64,
            .slow_inflict_seconds = 1.92,
            .extra_range = 60.0,
            .radial_pulse_damage = 4454.99,
            .radial_pulse_radius = 216.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 38880.0,
        .damage = 21870.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 2.86,
            .slow_inflict_seconds = 2.08,
            .extra_range = 65.0,
            .radial_pulse_damage = 13365.0,
            .radial_pulse_radius = 234.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 116640.0,
        .damage = 65610.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 3.19,
            .slow_inflict_seconds = 2.32,
            .extra_range = 72.5,
            .radial_pulse_damage = 40094.9,
            .radial_pulse_radius = 261.0
        }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 24.0003,
        .damage = 15.0003,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .move_speed_factor = 1.2275f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 72.0009,
        .damage = 45.0009,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .move_speed_factor = 1.2812f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 216.003,
        .damage = 135.003,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .move_speed_factor = 1.335f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 648.008,
        .damage = 405.008,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .move_speed_factor = 1.3887f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 1944.02,
        .damage = 1215.02,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .move_speed_factor = 1.4425f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 5832.07,
        .damage = 3645.07,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .move_speed_factor = 1.4963f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 17496.2,
        .damage = 10935.2,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .move_speed_factor = 1.55f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 54.0,
        .damage = 24.0003,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 20.9997, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 162.0,
        .damage = 72.0009,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 62.9991, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 486.0,
        .damage = 216.003,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 188.997, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 1458.0,
        .damage = 648.008,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 566.992, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 4374.0,
        .damage = 1944.02,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 1700.98, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 13122.0,
        .damage = 5832.07,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 5102.93, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 39366.0,
        .damage = 17496.2,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 15308.8, .time = 1.5 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 47.9997,
        .damage = 27.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 9.0, .time = 4.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 143.999,
        .damage = 81.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 27.0, .time = 4.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 431.997,
        .damage = 243.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 81.0, .time = 4.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 1295.99,
        .damage = 729.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 243.0, .time = 4.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 3887.98,
        .damage = 2187.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 729.0, .time = 4.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 11663.9,
        .damage = 6561.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 2187.0, .time = 4.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 34991.8,
        .damage = 19683.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 6561.0, .time = 4.0 } }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 29.9997,
        .damage = 47.9997,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 89.9991,
        .damage = 143.999,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 269.997,
        .damage = 431.997,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 809.992,
        .damage = 1295.99,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 2429.98,
        .damage = 3887.98,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 7289.93,
        .damage = 11663.9,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 21869.8,
        .damage = 34991.8,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 405.0,
        .damage = 135.003,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 1.8125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 1215.0,
        .damage = 405.008,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.6406,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 3645.0,
        .damage = 1215.02,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.4688,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 10935.0,
        .damage = 3645.07,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.2969,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 32805.0,
        .damage = 10935.2,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 135.003,
        .damage = 30.0024,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 15.0012, .on_hit_heal = 89.1 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 405.008,
        .damage = 90.0072,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 45.0036, .on_hit_heal = 267.3 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 1215.02,
        .damage = 270.022,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 77.9485, .on_hit_heal = 801.9 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 3645.07,
        .damage = 810.065,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 135.011, .on_hit_heal = 2405.7 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 10935.2,
        .damage = 2430.19,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 233.846, .on_hit_heal = 7217.1 }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 90.0,
        .damage = 56.9997,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .bonus_vs_poisoned = 0.45f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 270.0,
        .damage = 170.999,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .bonus_vs_poisoned = 0.5f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 810.0,
        .damage = 512.997,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .bonus_vs_poisoned = 0.55f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 2430.0,
        .damage = 1538.99,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .bonus_vs_poisoned = 0.6f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 7290.0,
        .damage = 4616.98,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .bonus_vs_poisoned = 0.65f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 21870.0,
        .damage = 13850.9,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .bonus_vs_poisoned = 0.7f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 65610.0,
        .damage = 41552.8,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .bonus_vs_poisoned = 0.75f }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 300.008,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.9688,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 900.023,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.7625,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 2700.07,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.5563,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 8100.21,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.35,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 14.9931,
        .damage = 8.991,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kMythic,
        .attributes = { .vision_factor = 0.5687f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 44.9793,
        .damage = 26.973,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kUltra,
        .attributes = { .vision_factor = 0.5125f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 134.938,
        .damage = 80.919,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kSuper,
        .attributes = { .vision_factor = 0.4563f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 404.814,
        .damage = 242.757,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kUnique,
        .attributes = { .vision_factor = 0.4f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 54.0,
        .damage = 54.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 9.0, .time = 2.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 162.0,
        .damage = 162.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 27.0, .time = 2.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 486.0,
        .damage = 486.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 81.0, .time = 2.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 1458.0,
        .damage = 1458.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 243.0, .time = 2.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 4374.0,
        .damage = 4374.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 729.0, .time = 2.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 13122.0,
        .damage = 13122.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 2187.0, .time = 2.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 39366.0,
        .damage = 39366.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 6561.0, .time = 2.0 } }
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 2430.0,
        .damage = 3402.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 2430.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 7290.0,
        .damage = 10206.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        // Wiki √3 armor ladder; runtime armor_tier_catchup → combat ×3.
        .attributes = { .armor = 4208.88, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 21870.0,
        .damage = 30618.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 7290.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 65610.0,
        .damage = 91854.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 12626.7, .icon_angle = 1.0 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 105.0,
        .damage = 27.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 7.5006, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 315.001,
        .damage = 81.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 22.5018, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 945.003,
        .damage = 243.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 67.5054, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 2835.01,
        .damage = 729.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 116.923, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 8505.02,
        .damage = 2187.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 202.516, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 25515.1,
        .damage = 6561.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 350.768, .defend_only = 1 }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 4860.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { 
            .extra_health = 4860.0, .damage_reflection = 0.275f }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 14580.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .extra_health = 8417.77, .damage_reflection = 0.31f }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 43740.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .extra_health = 14580.0, .damage_reflection = 0.345f }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 131220.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .extra_health = 25253.3, .damage_reflection = 0.38f }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 2430.0,
        .damage = 1890.01,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 1.98, .poison_damage = { .damage = 755.997, .time = 3.675 } }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 7290.0,
        .damage = 5670.02,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 2.16, .poison_damage = { .damage = 2267.99, .time = 3.85 } }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 21870.0,
        .damage = 17010.0,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 2.34, .poison_damage = { .damage = 6803.98, .time = 4.025 } }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 65610.0,
        .damage = 51030.1,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 2.61, .poison_damage = { .damage = 20411.9, .time = 4.375 } }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 65.9997,
        .damage = 33.0003,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 197.999,
        .damage = 99.0009,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 593.997,
        .damage = 297.003,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 1781.99,
        .damage = 891.008,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 5345.98,
        .damage = 2673.02,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 16037.9,
        .damage = 8019.07,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 48113.8,
        .damage = 24057.2,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 4320.01,
        .damage = 1079.99,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.385,
            .slow_inflict_seconds = 3.52,
            .aura_radius = 176.0,
            .aura_duration = 13.2,
            .aura_slow_seconds = 2.42,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 12960.0,
        .damage = 3239.97,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.42,
            .slow_inflict_seconds = 3.84,
            .aura_radius = 192.0,
            .aura_duration = 14.4,
            .aura_slow_seconds = 2.64,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 38880.0,
        .damage = 9719.9,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.455,
            .slow_inflict_seconds = 4.16,
            .aura_radius = 208.0,
            .aura_duration = 15.6,
            .aura_slow_seconds = 2.86,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 116640.0,
        .damage = 29159.7,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5075,
            .slow_inflict_seconds = 4.64,
            .aura_radius = 232.0,
            .aura_duration = 17.4,
            .aura_slow_seconds = 3.19,
            .defend_only = 1
        }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 150.004,
        .damage = 21.0033,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .move_speed_factor = 1.215f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 450.012,
        .damage = 63.0099,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .move_speed_factor = 1.2488f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 1350.04,
        .damage = 189.03,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .move_speed_factor = 1.2825f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 4050.11,
        .damage = 567.089,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .move_speed_factor = 1.3163f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 12150.3,
        .damage = 1701.27,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .move_speed_factor = 1.35f }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 1620.01,
        .damage = 648.008,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.6562,
            .constant_heal = 12.0042,
            .burst_heal = 1603.8,
            .extra_rotation_speed = 0.84
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 4860.02,
        .damage = 1944.02,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5875,
            .constant_heal = 20.7919,
            .burst_heal = 2777.86,
            .extra_rotation_speed = 0.96
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 14580.1,
        .damage = 5832.07,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.5187,
            .constant_heal = 36.0126,
            .burst_heal = 4811.4,
            .extra_rotation_speed = 1.08
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 43740.2,
        .damage = 17496.2,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.45,
            .constant_heal = 62.3757,
            .burst_heal = 8333.59,
            .extra_rotation_speed = 1.2
        }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 120.002,
        .damage = 35.9964,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 20.0, .poison_armor = 267.3 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 360.005,
        .damage = 107.989,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 20.0, .poison_armor = 801.9 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 1080.01,
        .damage = 323.968,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 20.0, .poison_armor = 1388.93 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 3240.04,
        .damage = 971.903,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 20.0, .poison_armor = 2405.7 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 9720.12,
        .damage = 2915.71,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 20.0, .poison_armor = 4166.79 }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 1350.0,
        .damage = 135.003,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 4050.01,
        .damage = 405.008,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 12150.0,
        .damage = 1215.02,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 36450.1,
        .damage = 3645.07,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 109350.0,
        .damage = 10935.2,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 120.002,
        .damage = 69.0039,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 17.9982, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 360.005,
        .damage = 207.012,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 53.9946, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 1080.01,
        .damage = 621.035,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 161.984, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 3240.04,
        .damage = 1863.11,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 485.951, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 9720.12,
        .damage = 5589.32,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 1457.85, .time = 5.0004 } }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 195.0,
        .damage = 29.9997,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 108.0, .damage_reflection = 0.27f }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 585.001,
        .damage = 89.9991,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 324.0, .damage_reflection = 0.305f }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 1755.0,
        .damage = 269.997,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 972.0, .damage_reflection = 0.34f }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 5265.01,
        .damage = 809.992,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 2916.0, .damage_reflection = 0.375f }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 15795.0,
        .damage = 2429.98,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 5050.66, .damage_reflection = 0.41f }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 47385.1,
        .damage = 7289.93,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 8748.0, .damage_reflection = 0.445f }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 142155.0,
        .damage = 21869.8,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 15152.0, .damage_reflection = 0.48f }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 45.0036,
        .damage = 74.9979,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_body_damage = 972.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 135.011,
        .damage = 224.994,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_body_damage = 2916.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 405.032,
        .damage = 674.981,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_body_damage = 8748.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 1215.1,
        .damage = 2024.94,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_body_damage = 8748.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 3645.29,
        .damage = 6074.83,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        // Was 78732 (one ×3 too high vs Ultra→Super ladder) — melted soft zones on contact.
        .attributes = { .extra_body_damage = 18000.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 135.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .dandelion_inflict_seconds = 11.875,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 405.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .dandelion_inflict_seconds = 13.5,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 1215.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .dandelion_inflict_seconds = 15.125,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 3645.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .dandelion_inflict_seconds = 16.75,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 10935.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .dandelion_inflict_seconds = 18.375,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 32805.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .dandelion_inflict_seconds = 20.0,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 254.996,
        .damage = 89.9991,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.2f, .bonus_vs_slowed = 0.4f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 764.988,
        .damage = 269.997,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.22f, .bonus_vs_slowed = 0.4375f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 2294.96,
        .damage = 809.992,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.24f, .bonus_vs_slowed = 0.475f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 6884.89,
        .damage = 2429.98,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.26f, .bonus_vs_slowed = 0.5125f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 20654.7,
        .damage = 7289.93,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.28f, .bonus_vs_slowed = 0.55f }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 479.998,
        .damage = 15.0012,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 4.5036, .extra_health = 1782.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 1439.99,
        .damage = 45.0036,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 13.5108, .extra_health = 5346.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 4319.98,
        .damage = 135.011,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 23.4014, .extra_health = 9259.54 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 12959.9,
        .damage = 405.032,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 40.5324, .extra_health = 16038.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 38879.8,
        .damage = 1215.1,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 70.2042, .extra_health = 27778.6 }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 1620.0,
        .damage = 81.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { 
            .secondary_reload = 1.8125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 4860.0,
        .damage = 243.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { 
            .secondary_reload = 1.6406,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 14580.0,
        .damage = 729.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .secondary_reload = 1.4688,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 43740.0,
        .damage = 2187.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .secondary_reload = 1.2969,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 131220.0,
        .damage = 6561.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .secondary_reload = 1.125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 120.0,
        .damage = 15.0003,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 24.75 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 359.999,
        .damage = 45.0009,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 74.25 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 1080.0,
        .damage = 135.003,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 222.75 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 3239.99,
        .damage = 405.008,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 668.25 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 9719.98,
        .damage = 1215.02,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 1157.44 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 29159.9,
        .damage = 3645.07,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 2004.75 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 87479.8,
        .damage = 10935.2,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 3472.33 }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 60.0003,
        .damage = 65.9997,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .burn_damage = { .damage = 11.9997, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 180.001,
        .damage = 197.999,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .burn_damage = { .damage = 35.9991, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 540.003,
        .damage = 593.997,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .burn_damage = { .damage = 107.997, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 1620.01,
        .damage = 1781.99,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .burn_damage = { .damage = 323.992, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 4860.02,
        .damage = 5345.98,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .burn_damage = { .damage = 971.976, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 14580.1,
        .damage = 16037.9,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .burn_damage = { .damage = 2915.93, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 43740.2,
        .damage = 48113.8,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .burn_damage = { .damage = 8747.78, .time = 3.0 } }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 179.998,
        .damage = 102.003,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.45, .extra_range = 24.0003 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 539.995,
        .damage = 306.01,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.525, .extra_range = 72.0009 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 1619.98,
        .damage = 918.03,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.6, .extra_range = 144.0018 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 4859.95,
        .damage = 2754.09,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.675, .extra_range = 288.0036 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 14579.9,
        .damage = 8262.27,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.75, .extra_range = 576.0072 }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 164.997,
        .damage = 144.002,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.16f, .armor_penetration = 0.4f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 494.991,
        .damage = 432.005,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.18f, .armor_penetration = 0.4375f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 1484.97,
        .damage = 1296.02,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.2f, .armor_penetration = 0.475f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 4454.92,
        .damage = 3888.05,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.22f, .armor_penetration = 0.5125f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 13364.8,
        .damage = 11664.1,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.24f, .armor_penetration = 0.55f }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 74.9979,
        .damage = 72.0009,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 648.0, .slow_inflict_seconds = 2.7, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 224.994,
        .damage = 216.003,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 1944.0, .slow_inflict_seconds = 3.025, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 674.981,
        .damage = 648.008,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 3367.11, .slow_inflict_seconds = 3.35, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 2024.94,
        .damage = 1944.02,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 5832.0, .slow_inflict_seconds = 3.675, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 6074.83,
        .damage = 5832.07,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 10101.3, .slow_inflict_seconds = 4.0, .defend_only = 1 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 450.004,
        .damage = 15.0012,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_armor = 356.4, .extra_health = 2592.0 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 1350.01,
        .damage = 45.0036,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_armor = 1069.2, .extra_health = 7776.0 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 4050.03,
        .damage = 135.011,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_armor = 1851.91, .extra_health = 13468.4 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 12150.1,
        .damage = 405.032,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_armor = 3207.6, .extra_health = 23328.0 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 36450.3,
        .damage = 1215.1,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_armor = 5555.73, .extra_health = 40405.3 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 150.004,
        .damage = 126.004,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 2.7, .extra_health = 972.0 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 450.012,
        .damage = 378.011,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 3.025, .extra_health = 2916.0 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 1350.04,
        .damage = 1134.03,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 3.35, .extra_health = 5050.66 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 4050.11,
        .damage = 3402.1,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 3.675, .extra_health = 8748.0 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 12150.3,
        .damage = 10206.3,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 4.0, .extra_health = 15152.0 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 135.003,
        .damage = 78.003,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 405.008,
        .damage = 234.009,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 1215.02,
        .damage = 702.027,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 3645.07,
        .damage = 2106.08,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 10935.2,
        .damage = 6318.24,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 105.0,
        .damage = 48.0006,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.3625,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 315.001,
        .damage = 144.002,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.3281,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 945.003,
        .damage = 432.005,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.2937,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 2835.01,
        .damage = 1296.02,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.2594,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 8505.02,
        .damage = 3888.05,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.225,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 945.003,
        .damage = 26.9973,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 1.8125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 2835.01,
        .damage = 80.9919,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.6406,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 8505.02,
        .damage = 242.976,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.4688,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 25515.1,
        .damage = 728.927,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.2969,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 76545.2,
        .damage = 2186.78,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 54.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 18.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 162.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 54.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 486.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 162.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 1458.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 486.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 4374.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 841.777, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 13122.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 1458.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 39366.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 2525.33, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 11340.0,
        .damage = 323.992,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 593.989, .extra_health = 14040.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 34020.0,
        .damage = 971.976,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 1028.82, .extra_health = 24318.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 102060.0,
        .damage = 2915.93,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 1781.97, .extra_health = 42120.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 306180.0,
        .damage = 8747.78,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 3086.46, .extra_health = 72954.0, .defend_only = 1 }
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 199.997,
        .damage = 10.0035,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 599.991,
        .damage = 30.0105,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 1799.97,
        .damage = 90.0315,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 5399.92,
        .damage = 270.094,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 16199.8,
        .damage = 810.284,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 420.001,
        .damage = 81.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 267.3, .extra_rotation_speed = 0.45 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 1260.0,
        .damage = 243.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 801.9, .extra_rotation_speed = 0.525 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 3780.01,
        .damage = 729.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 1388.93, .extra_rotation_speed = 0.6 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 11340.0,
        .damage = 2187.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 2405.7, .extra_rotation_speed = 0.675 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 34020.1,
        .damage = 6561.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 4166.79, .extra_rotation_speed = 0.75 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 210.001,
        .damage = 74.9979,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.725, .burst_heal = 2916.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 630.002,
        .damage = 224.994,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.6562, .burst_heal = 8748.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 1890.01,
        .damage = 674.981,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.5875, .burst_heal = 15152.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 5670.02,
        .damage = 2024.94,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.5187, .burst_heal = 26244.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 17010.0,
        .damage = 6074.83,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.45, .burst_heal = 45455.9, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 216.0,
        .damage = 162.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.7937, .burst_heal = 540.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 648.0,
        .damage = 486.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.725, .burst_heal = 1620.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 1944.0,
        .damage = 1458.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.6562, .burst_heal = 4860.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 5832.0,
        .damage = 4374.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.5875, .burst_heal = 8417.77, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 17496.0,
        .damage = 13122.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.5187, .burst_heal = 14580.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 52488.0,
        .damage = 39366.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.45, .burst_heal = 25253.3, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 120.002,
        .damage = 15.0012,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.725, .constant_heal = 8.9991, .burst_heal = 1296.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 360.005,
        .damage = 45.0036,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.6562, .constant_heal = 26.9973, .burst_heal = 3888.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 1080.01,
        .damage = 135.011,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.5875, .constant_heal = 46.7607, .burst_heal = 6734.21, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 3240.04,
        .damage = 405.032,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.5187, .constant_heal = 80.9919, .burst_heal = 11664.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 9720.12,
        .damage = 1215.1,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.45, .constant_heal = 140.282, .burst_heal = 20202.6, .defend_only = 1 }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 89.9991,
        .damage = 11.9961,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .constant_heal = 6.0021,
            .poison_damage = { .damage = 8.9991, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 269.997,
        .damage = 35.9883,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .constant_heal = 18.0063,
            .poison_damage = { .damage = 26.9973, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 809.992,
        .damage = 107.965,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .constant_heal = 31.1878,
            .poison_damage = { .damage = 80.9919, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 2429.98,
        .damage = 323.895,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .constant_heal = 54.0189,
            .poison_damage = { .damage = 242.976, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 7289.93,
        .damage = 971.684,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .constant_heal = 93.5635,
            .poison_damage = { .damage = 728.927, .time = 2.9997 }
        }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 329.999,
        .damage = 18.0009,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 270.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 989.998,
        .damage = 54.0027,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 810.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 2969.99,
        .damage = 162.008,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 2430.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 8909.98,
        .damage = 486.024,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 4208.88, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 26730.0,
        .damage = 1458.07,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 7290.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 80189.9,
        .damage = 4374.22,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 12626.7, .defend_only = 1 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 65.9997,
        .damage = 60.0003,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 1.5375 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 197.999,
        .damage = 180.001,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 1.7812 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 593.997,
        .damage = 540.003,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 2.025 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 1781.99,
        .damage = 1620.01,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 2.2687 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 5345.98,
        .damage = 4860.02,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 2.5125 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 16037.9,
        .damage = 14580.1,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 2.7563 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 48113.8,
        .damage = 43740.2,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 3.0 }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 150.004,
        .damage = 54.0027,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.175,
            .slow_inflict_seconds = 0.6075,
            .extra_range = 74.9979,
            .radial_pulse_damage = 54.0027,
            .radial_pulse_radius = 330.0021
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 450.012,
        .damage = 162.008,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.9688,
            .slow_inflict_seconds = 0.6806,
            .extra_range = 224.9937,
            .radial_pulse_damage = 162.008,
            .radial_pulse_radius = 990.0063
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 1350.04,
        .damage = 486.024,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.7625,
            .slow_inflict_seconds = 0.7537,
            .extra_range = 449.9874,
            .radial_pulse_damage = 486.024,
            .radial_pulse_radius = 1980.0126
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 4050.11,
        .damage = 1458.07,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.5563,
            .slow_inflict_seconds = 0.8269,
            .extra_range = 899.9748,
            .radial_pulse_damage = 1458.07,
            .radial_pulse_radius = 3960.0252
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 12150.3,
        .damage = 4374.22,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.35,
            .slow_inflict_seconds = 0.9,
            .extra_range = 1799.9496,
            .radial_pulse_damage = 4374.22,
            .radial_pulse_radius = 7920.0504
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 7560.0,
        .damage = 2565.01,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_health = 64800.0,
            .burn_damage = { .damage = 486.0, .time = 5.25 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 22680.0,
        .damage = 7695.03,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_health = 112237.0,
            .burn_damage = { .damage = 1458.0, .time = 5.5 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 68040.0,
        .damage = 23085.1,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_health = 194400.0,
            .burn_damage = { .damage = 4374.0, .time = 5.75 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 204120.0,
        .damage = 69255.3,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_health = 336711.0,
            .burn_damage = { .damage = 13122.0, .time = 6.25 }
        }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 108.0,
        .damage = 16.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 4.95 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 324.0,
        .damage = 49.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 14.85 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 972.0,
        .damage = 148.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 44.55 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 2916.0,
        .damage = 445.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 133.65 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 8748.0,
        .damage = 1336.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 400.95 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 26244.0,
        .damage = 4009.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 694.466 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 78732.0,
        .damage = 12028.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 1202.85 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 236196.0,
        .damage = 36085.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 2083.4 }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 119.999,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.2362f }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 359.996,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.265f }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 1079.99,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.2937f }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 3239.97,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.3225f }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 9719.9,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.3512f }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 29159.7,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.38f }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 16.5,
        .damage = 21.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 49.5,
        .damage = 63.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 148.5,
        .damage = 189.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 445.5,
        .damage = 567.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 1336.5,
        .damage = 1701.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 4009.5,
        .damage = 5103.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 12028.5,
        .damage = 15309.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 36085.5,
        .damage = 45927.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 24300.0,
        .damage = 16200.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_damage_factor = 1.6000f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 72900.0,
        .damage = 48600.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_damage_factor = 1.7700f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 218700.0,
        .damage = 145800.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_damage_factor = 2.0000f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 656100.0,
        .damage = 437400.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_damage_factor = 2.3000f
        }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 75.0,
        .damage = 36.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 225.0,
        .damage = 108.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 675.0,
        .damage = 324.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 2025.0,
        .damage = 972.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 6075.0,
        .damage = 2916.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 18225.0,
        .damage = 8748.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 54675.0,
        .damage = 26244.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 164025.0,
        .damage = 78732.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 150.004,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 11.9961, .extra_health = 1215.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 450.012,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 35.9883, .extra_health = 3645.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 1350.04,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 62.3336, .extra_health = 6313.33, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 4050.11,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 107.965, .extra_health = 10935.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 12150.3,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 187.001, .extra_health = 18940.0, .defend_only = 1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 19.9989,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 81.0, .poison_armor = 486.0, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 59.9967,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 243.0, .poison_armor = 1458.0, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 179.99,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 420.888, .poison_armor = 2525.33, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 539.97,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 729.0, .poison_armor = 4374.0, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 1619.91,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 1262.67, .poison_armor = 7575.99, .icon_angle = 0.1 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 675.0,
        .damage = 251.999,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 270.0, .burn_damage = { .damage = 18.0009, .time = 3.0 }, .splash_damage = 324.0, .splash_radius = 1890.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 2025.0,
        .damage = 755.997,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 810.0, .burn_damage = { .damage = 54.0027, .time = 3.0 }, .splash_damage = 972.0, .splash_radius = 5670.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 6075.0,
        .damage = 2267.99,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 2430.0, .burn_damage = { .damage = 162.008, .time = 3.0 }, .splash_damage = 2916.0, .splash_radius = 17010.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 18225.0,
        .damage = 6803.98,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 4208.88, .burn_damage = { .damage = 486.024, .time = 3.0 }, .splash_damage = 8748.0, .splash_radius = 34020.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 54675.0,
        .damage = 20411.9,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 7290.0, .burn_damage = { .damage = 1458.07, .time = 3.0 }, .splash_damage = 26244.0, .splash_radius = 68040.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 164025.0,
        .damage = 61235.8,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 12626.7, .burn_damage = { .damage = 4374.22, .time = 3.0 }, .splash_damage = 78732.0, .splash_radius = 136080.0 }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 29.9997,
        .damage = 15.0003,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .magnet_radius = 925.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 89.9991,
        .damage = 45.0009,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .magnet_radius = 1137.5, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 269.997,
        .damage = 135.003,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .magnet_radius = 1350.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 809.992,
        .damage = 405.008,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .magnet_radius = 1562.5, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 2429.98,
        .damage = 1215.02,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .magnet_radius = 1775.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 7289.93,
        .damage = 3645.07,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .magnet_radius = 1987.5, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 21869.8,
        .damage = 10935.2,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .magnet_radius = 2200.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 81.0,
        .damage = 81.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 243.0,
        .damage = 243.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 729.0,
        .damage = 729.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 2187.0,
        .damage = 2187.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 6561.0,
        .damage = 6561.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 720.001,
        .damage = 180.001,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 0.27 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 2160.0,
        .damage = 540.003,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.324 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 6480.01,
        .damage = 1620.01,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.378 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 19440.0,
        .damage = 4860.02,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.432 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 58320.1,
        .damage = 14580.1,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.486 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 174960.0,
        .damage = 43740.2,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.54 }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 105.0,
        .damage = 48.0006,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_rotation_speed = 0.252,
            .poison_damage = { .damage = 8.9991, .time = 2.0007 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 315.001,
        .damage = 144.002,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_rotation_speed = 0.294,
            .poison_damage = { .damage = 26.9973, .time = 2.0007 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 945.003,
        .damage = 432.005,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_rotation_speed = 0.336,
            .poison_damage = { .damage = 80.9919, .time = 2.0007 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 2835.01,
        .damage = 1296.02,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_rotation_speed = 0.378,
            .poison_damage = { .damage = 242.976, .time = 2.0007 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 8505.02,
        .damage = 3888.05,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_rotation_speed = 0.42,
            .poison_damage = { .damage = 728.927, .time = 2.0007 }
        }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 120.002,
        .damage = 120.002,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .burn_damage = { .damage = 486.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 360.005,
        .damage = 360.005,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .burn_damage = { .damage = 1458.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 1080.01,
        .damage = 1080.01,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .burn_damage = { .damage = 4374.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 3240.04,
        .damage = 3240.04,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .burn_damage = { .damage = 13122.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 9720.12,
        .damage = 9720.12,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .burn_damage = { .damage = 39366.0, .time = 3.0 } }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 36.0126,
        .damage = 15.0174,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kUltra,
        .attributes = { .vision_factor = 0.2725f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 108.038,
        .damage = 45.0522,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kSuper,
        .attributes = { .vision_factor = 0.2263f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 324.113,
        .damage = 135.157,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kUnique,
        .attributes = { .vision_factor = 0.18f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 216.0,
        .damage = 305.999,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor_penetration = 0.5938f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 648.0,
        .damage = 917.997,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor_penetration = 0.625f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 1944.0,
        .damage = 2753.99,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor_penetration = 0.6562f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 5832.0,
        .damage = 8261.98,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor_penetration = 0.6875f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 17496.0,
        .damage = 24785.9,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor_penetration = 0.7188f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 52488.0,
        .damage = 74357.8,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor_penetration = 0.75f }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 720.001,
        .damage = 24.0003,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.24f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 2160.0,
        .damage = 72.0009,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.275f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 6480.01,
        .damage = 216.003,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.31f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 19440.0,
        .damage = 648.008,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.345f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 58320.1,
        .damage = 1944.02,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.38f, .defend_only = 1 }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 405.0,
        .damage = 648.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.3625,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 1215.0,
        .damage = 1944.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.3281,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 3645.0,
        .damage = 5832.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.2937,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 10935.0,
        .damage = 17496.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.2594,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 32805.0,
        .damage = 52488.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.225,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 210.001,
        .damage = 83.997,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 324.0, .extra_range = 89.9991 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 630.002,
        .damage = 251.991,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 972.0, .extra_range = 269.9973 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 1890.01,
        .damage = 755.973,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 1683.55, .extra_range = 539.9946 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 5670.02,
        .damage = 2267.92,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 2916.0, .extra_range = 1079.9892 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 17010.0,
        .damage = 6803.76,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 5050.66, .extra_range = 2159.9784 }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 27.0,
        .damage = 40.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.204f
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 81.0,
        .damage = 121.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.232f
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 243.0,
        .damage = 364.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.26f
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 729.0,
        .damage = 1093.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.288f
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 2187.0,
        .damage = 3280.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.316f
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 6561.0,
        .damage = 9841.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.344f
        }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 83.9997,
        .damage = 29.9997,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 0.48 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 251.999,
        .damage = 89.9991,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.576 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 755.997,
        .damage = 269.997,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.672 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 2267.99,
        .damage = 809.992,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.768 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 6803.98,
        .damage = 2429.98,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.864 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 20411.9,
        .damage = 7289.93,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.96 }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 29.9997,
        .damage = 29.9997,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .slow_inflict_seconds = 2.375,
            .poison_damage = {
                .damage = 15.0012,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 89.9991,
        .damage = 89.9991,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .slow_inflict_seconds = 2.7,
            .poison_damage = {
                .damage = 45.0036,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 269.997,
        .damage = 269.997,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .slow_inflict_seconds = 3.025,
            .poison_damage = {
                .damage = 135.011,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 809.992,
        .damage = 809.992,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .slow_inflict_seconds = 3.35,
            .poison_damage = {
                .damage = 405.032,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 2429.98,
        .damage = 2429.98,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .slow_inflict_seconds = 3.675,
            .poison_damage = {
                .damage = 1215.1,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 7289.93,
        .damage = 7289.93,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .slow_inflict_seconds = 4.0,
            .poison_damage = {
                .damage = 3645.29,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 164.997,
        .damage = 24.0003,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.175,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 494.991,
        .damage = 72.0009,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.9688,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 1484.97,
        .damage = 216.003,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.7625,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 4454.92,
        .damage = 648.008,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.5563,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 13364.8,
        .damage = 1944.02,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.35,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 108.0,
        .damage = 36.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 36.0, .time = 4.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 324.0,
        .damage = 108.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 108.0, .time = 4.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 972.0,
        .damage = 324.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 324.0, .time = 4.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 2916.0,
        .damage = 972.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 972.0, .time = 4.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 8748.0,
        .damage = 2916.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 2916.0, .time = 4.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 26244.0,
        .damage = 8748.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 8748.0, .time = 4.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 78732.0,
        .damage = 26244.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 26244.0, .time = 4.0 } }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 405.0,
        .damage = 648.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.3625,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 1215.0,
        .damage = 1944.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.3281,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 3645.0,
        .damage = 5832.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.2937,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 10935.0,
        .damage = 17496.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.2594,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 32805.0,
        .damage = 52488.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.225,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 120.002,
        .damage = 24.0003,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.3625, .slow_inflict_seconds = 4.05, .aura_heal = 8.9991, .aura_radius = 284.9985, .aura_duration = 6.7, .defend_only = 1 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 360.005,
        .damage = 72.0009,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.3281, .slow_inflict_seconds = 4.5375, .aura_heal = 26.9973, .aura_radius = 854.9955, .aura_duration = 7.825, .defend_only = 1 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 1080.01,
        .damage = 216.003,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.2937, .slow_inflict_seconds = 5.025, .aura_heal = 46.7607, .aura_radius = 1709.991, .aura_duration = 8.95, .defend_only = 1 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 3240.04,
        .damage = 648.008,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.2594, .slow_inflict_seconds = 5.5125, .aura_heal = 80.9919, .aura_radius = 3419.982, .aura_duration = 10.075, .defend_only = 1 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 9720.12,
        .damage = 1944.02,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.225, .slow_inflict_seconds = 6.0, .aura_heal = 140.282, .aura_radius = 6839.964, .aura_duration = 11.2, .defend_only = 1 }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 2024.99,
        .damage = 405.008,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 3.2812,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 6074.98,
        .damage = 1215.02,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 2.9375,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 18224.9,
        .damage = 3645.07,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 2.5938,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 54674.8,
        .damage = 10935.2,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 2.25,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 65.9988,
        .damage = 24.0003,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 2.375 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 197.996,
        .damage = 72.0009,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 2.7 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 593.989,
        .damage = 216.003,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 3.025 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 1781.97,
        .damage = 648.008,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 3.35 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 5345.9,
        .damage = 1944.02,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 3.675 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 16037.7,
        .damage = 5832.07,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 4.0 }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 135.003,
        .damage = 216.003,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 405.008,
        .damage = 648.008,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 1215.02,
        .damage = 1944.02,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 3645.07,
        .damage = 5832.07,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 10935.2,
        .damage = 17496.2,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 359.999,
        .damage = 24.0003,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 18.0009, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 1080.0,
        .damage = 72.0009,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 54.0027, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 3239.99,
        .damage = 216.003,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 162.008, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 9719.98,
        .damage = 648.008,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 486.024, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 29159.9,
        .damage = 1944.02,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 1458.07, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 87479.8,
        .damage = 5832.07,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 4374.22, .time = 2.0 } }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 42.0003,
        .damage = 16.4997,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_rotation_speed = 0.42
        }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 126.001,
        .damage = 49.4991,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_rotation_speed = 0.525
        }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 378.003,
        .damage = 148.497,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_rotation_speed = 0.63
        }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 1134.01,
        .damage = 445.492,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_rotation_speed = 0.735
        }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 3402.02,
        .damage = 1336.48,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_rotation_speed = 0.84
        }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 10206.1,
        .damage = 4009.43,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_rotation_speed = 0.945
        }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 30618.2,
        .damage = 12028.3,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_rotation_speed = 1.05
        }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 540.003,
        .damage = 81.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 445.5, .extra_health = 1113.75 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 1620.01,
        .damage = 243.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 1336.5, .extra_health = 3341.25 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 4860.02,
        .damage = 729.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 2314.89, .extra_health = 5787.21 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 14580.1,
        .damage = 2187.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 4009.5, .extra_health = 10023.8 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 43740.2,
        .damage = 6561.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 6944.66, .extra_health = 17361.6 }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 27.0,
        .damage = 27.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 24.0003, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 81.0,
        .damage = 81.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 72.0009, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 243.0,
        .damage = 243.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 216.003, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 729.0,
        .damage = 729.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 648.008, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 2187.0,
        .damage = 2187.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 1944.02, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 6561.0,
        .damage = 6561.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 5832.07, .time = 3.0 }
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 27.0,
        .damage = 59.9994,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_range = 100.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 81.0,
        .damage = 179.998,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_range = 100.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 243.0,
        .damage = 539.995,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_range = 100.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 729.0,
        .damage = 1619.98,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_range = 100.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 2187.0,
        .damage = 4859.95,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_range = 100.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 6561.0,
        .damage = 14579.9,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_range = 100.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 164.997,
        .damage = 107.997,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_damage_factor = 1.24f,
            .execute_threshold = 0.35f,
            .execute_factor = 1.88f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 494.991,
        .damage = 323.992,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_damage_factor = 1.26f,
            .execute_threshold = 0.375f,
            .execute_factor = 1.95f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 1484.97,
        .damage = 971.976,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_damage_factor = 1.28f,
            .execute_threshold = 0.4f,
            .execute_factor = 2.02f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 4454.92,
        .damage = 2915.93,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_damage_factor = 1.3f,
            .execute_threshold = 0.425f,
            .execute_factor = 2.09f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 13364.8,
        .damage = 8747.78,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_damage_factor = 1.32f,
            .execute_threshold = 0.45f,
            .execute_factor = 2.16f
        }
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 16.5,
        .damage = 19.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 49.5,
        .damage = 58.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 148.5,
        .damage = 175.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 445.5,
        .damage = 526.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 1336.5,
        .damage = 1579.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 4009.5,
        .damage = 4738.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 12028.5,
        .damage = 14215.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Rust Needle",
        .description = "Fast rusty needles that poke often",
        .health = 36085.5,
        .damage = 42646.5,
        .radius = 10.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 8
        }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 29.9997,
        .damage = 35.9991,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.225f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 89.9991,
        .damage = 107.997,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.26f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 269.997,
        .damage = 323.992,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.295f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 809.992,
        .damage = 971.976,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.33f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 2429.98,
        .damage = 2915.93,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.365f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 7289.93,
        .damage = 8747.78,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.4f }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 29.9997,
        .damage = 27.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 89.9991,
        .damage = 81.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 269.997,
        .damage = 243.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 809.992,
        .damage = 729.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 2429.98,
        .damage = 2187.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 7289.93,
        .damage = 6561.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 59.9967,
        .damage = 50.9976,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 1.45,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 179.99,
        .damage = 152.993,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.3125,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 539.97,
        .damage = 458.978,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.175,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 1619.91,
        .damage = 1376.94,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.0375,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 4859.73,
        .damage = 4130.81,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.9,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 96.0003,
        .damage = 47.9997,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 288.001,
        .damage = 143.999,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 864.003,
        .damage = 431.997,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 2592.01,
        .damage = 1295.99,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 7776.02,
        .damage = 3887.98,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 23328.1,
        .damage = 11663.9,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 69984.2,
        .damage = 34991.8,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 389.999,
        .damage = 30.0024,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .damage_reflection = 0.29f,
            .burn_damage = { .damage = 405.0, .time = 1.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 1170.0,
        .damage = 90.0072,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .damage_reflection = 0.325f,
            .burn_damage = { .damage = 1215.0, .time = 1.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 3509.99,
        .damage = 270.022,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .damage_reflection = 0.36f,
            .burn_damage = { .damage = 3645.0, .time = 1.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 10530.0,
        .damage = 810.065,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .damage_reflection = 0.395f,
            .burn_damage = { .damage = 10935.0, .time = 1.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 31589.9,
        .damage = 2430.19,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .damage_reflection = 0.43f,
            .burn_damage = { .damage = 32805.0, .time = 1.0 }
        }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 156.0,
        .damage = 24.0003,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 19.8 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 467.999,
        .damage = 72.0009,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 59.4 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 1404.0,
        .damage = 216.003,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 178.2 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 4211.99,
        .damage = 648.008,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 534.6 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 12636.0,
        .damage = 1944.02,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 925.954 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 37907.9,
        .damage = 5832.07,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 1603.8 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 113724.0,
        .damage = 17496.2,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 2777.86 }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 3239.99,
        .damage = 3780.01,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.7000f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 9719.98,
        .damage = 11340.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.8700f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 29159.9,
        .damage = 34020.1,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 2.1000f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 87479.8,
        .damage = 102060.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 2.4000f }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 135.0,
        .damage = 33.0003,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 405.0,
        .damage = 99.0009,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 1215.0,
        .damage = 297.003,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 3645.0,
        .damage = 891.008,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 10935.0,
        .damage = 2673.02,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 32805.0,
        .damage = 8019.07,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 98415.0,
        .damage = 24057.2,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 359.996,
        .damage = 8.9991,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 1.8125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 1079.99,
        .damage = 26.9973,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.6406,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 3239.97,
        .damage = 80.9919,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.4688,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 9719.9,
        .damage = 242.976,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.2969,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 29159.7,
        .damage = 728.927,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 27.0,
        .damage = 51.0003,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 81.0,
        .damage = 153.001,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 243.0,
        .damage = 459.003,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 729.0,
        .damage = 1377.01,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 2187.0,
        .damage = 4131.02,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 6561.0,
        .damage = 12393.1,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 324.0,
        .damage = 135.003,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 17.9982, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 972.0,
        .damage = 405.008,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 53.9946, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 2916.0,
        .damage = 1215.02,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 161.984, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 8748.0,
        .damage = 3645.07,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 485.951, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 26244.0,
        .damage = 10935.2,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 1457.85, .time = 3.0 } }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 119.999,
        .damage = 45.0009,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.9797f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 359.996,
        .damage = 135.003,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.0563f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 1079.99,
        .damage = 405.008,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.1328f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 3239.97,
        .damage = 1215.02,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.2094f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 9719.9,
        .damage = 3645.07,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.2859f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 29159.7,
        .damage = 10935.2,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.3625f }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 450.001,
        .damage = 288.001,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 162.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 1350.0,
        .damage = 864.003,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 486.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 4050.01,
        .damage = 2592.01,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 1458.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 12150.0,
        .damage = 7776.02,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 2525.33 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 36450.1,
        .damage = 23328.1,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 4374.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 109350.0,
        .damage = 69984.2,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 7575.99 }
    },
    {
        .name = "Stick",
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 2430.0,
        .damage = 243.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Stick",
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 7290.0,
        .damage = 729.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Stick",
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 21870.0,
        .damage = 2187.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Stick",
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 65610.0,
        .damage = 6561.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 72.0,
        .damage = 10.5003,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .armor = 49.5
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 216.0,
        .damage = 31.5009,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .armor = 148.5
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 648.0,
        .damage = 94.5027,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .armor = 445.5
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 1944.0,
        .damage = 283.508,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .armor = 1336.5
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 5832.0,
        .damage = 850.524,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .armor = 2314.89
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 17496.0,
        .damage = 2551.57,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .armor = 4009.5
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 52488.0,
        .damage = 7654.72,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .armor = 6944.66
        }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 569.997,
        .damage = 15.0012,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 405.0, .poison_armor = 801.9 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 1709.99,
        .damage = 45.0036,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 1215.0, .poison_armor = 2405.7 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 5129.97,
        .damage = 135.011,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 2104.44, .poison_armor = 4166.79 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 15389.9,
        .damage = 405.032,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 3645.0, .poison_armor = 7217.1 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 46169.8,
        .damage = 1215.1,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 6313.33, .poison_armor = 12500.4 }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 30.0348,
        .damage = 15.0174,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_range = 96.25, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 90.1044,
        .damage = 45.0522,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_range = 108.125, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 270.313,
        .damage = 135.157,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_range = 120.0, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 269.997,
        .damage = 72.0009,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.29f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 809.992,
        .damage = 216.003,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.325f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 2429.98,
        .damage = 648.008,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.36f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 7289.93,
        .damage = 1944.02,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.395f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 21869.8,
        .damage = 5832.07,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.43f }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 45.0009,
        .damage = 32.9994,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 135.003,
        .damage = 98.9982,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 405.008,
        .damage = 296.995,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 1215.02,
        .damage = 890.984,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 3645.07,
        .damage = 2672.95,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 10935.2,
        .damage = 8018.85,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 29.9997,
        .damage = 34.5006,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 1.7812,
            .poison_damage = { .damage = 11.9988, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 89.9991,
        .damage = 103.502,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 2.025,
            .poison_damage = { .damage = 35.9964, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 269.997,
        .damage = 310.505,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 2.2687,
            .poison_damage = { .damage = 107.989, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 809.992,
        .damage = 931.516,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 2.5125,
            .poison_damage = { .damage = 323.968, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 2429.98,
        .damage = 2794.55,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 2.7563,
            .poison_damage = { .damage = 971.903, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 7289.93,
        .damage = 8383.65,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 3.0,
            .poison_damage = { .damage = 2915.71, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 165.0,
        .damage = 36.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 1.1275, .snare_seconds = 0.82 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 494.999,
        .damage = 108.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 1.3063, .snare_seconds = 0.95 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 1485.0,
        .damage = 324.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 1.485, .snare_seconds = 1.08 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 4454.99,
        .damage = 972.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 1.6638, .snare_seconds = 1.21 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 13365.0,
        .damage = 2916.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 1.8425, .snare_seconds = 1.34 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 40094.9,
        .damage = 8748.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 2.0213, .snare_seconds = 1.47 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 120285.0,
        .damage = 26244.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 2.2, .snare_seconds = 1.6 }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 83.9997,
        .damage = 35.9991,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 1.9844,
            .extra_range = 54.0,
            .radial_pulse_damage = 42.0012,
            .radial_pulse_radius = 270.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 251.999,
        .damage = 107.997,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 1.8125,
            .extra_range = 162.0,
            .radial_pulse_damage = 126.004,
            .radial_pulse_radius = 810.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 755.997,
        .damage = 323.992,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.6406,
            .extra_range = 486.0,
            .radial_pulse_damage = 378.011,
            .radial_pulse_radius = 2430.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 2267.99,
        .damage = 971.976,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.4688,
            .extra_range = 972.0,
            .radial_pulse_damage = 1134.03,
            .radial_pulse_radius = 4860.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 6803.98,
        .damage = 2915.93,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.2969,
            .extra_range = 1944.0,
            .radial_pulse_damage = 3402.1,
            .radial_pulse_radius = 9720.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 20411.9,
        .damage = 8747.78,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.125,
            .extra_range = 3888.0,
            .radial_pulse_damage = 10206.3,
            .radial_pulse_radius = 19440.0
        }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 54.0,
        .damage = 27.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 118.8 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 162.0,
        .damage = 81.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 356.4 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 486.0,
        .damage = 243.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 1069.2 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 1458.0,
        .damage = 729.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 1851.91 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 4374.0,
        .damage = 2187.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 3207.6 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 13122.0,
        .damage = 6561.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 5555.73 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 779.998,
        .damage = 24.0003,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 1296.0, .extra_health = 1458.0, .taunt_radius = 260.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 2339.99,
        .damage = 72.0009,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 3888.0, .extra_health = 4374.0, .taunt_radius = 295.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 7019.98,
        .damage = 216.003,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 6734.21, .extra_health = 7575.99, .taunt_radius = 330.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 21059.9,
        .damage = 648.008,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 11664.0, .extra_health = 13122.0, .taunt_radius = 365.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 63179.8,
        .damage = 1944.02,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 20202.6, .extra_health = 22728.0, .taunt_radius = 400.0 }
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 405.0,
        .damage = 648.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 1215.0,
        .damage = 1944.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 3645.0,
        .damage = 5832.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 10935.0,
        .damage = 17496.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 32805.0,
        .damage = 52488.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 285.001,
        .damage = 35.9991,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mass = 88.5, .armor = 44.55 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 855.004,
        .damage = 107.997,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mass = 106.0, .armor = 133.65 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 2565.01,
        .damage = 323.992,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mass = 123.5, .armor = 400.95 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 7695.03,
        .damage = 971.976,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mass = 141.0, .armor = 694.466 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 23085.1,
        .damage = 2915.93,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mass = 158.5, .armor = 1202.85 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 69255.3,
        .damage = 8747.78,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mass = 176.0, .armor = 2083.4 }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 269.997,
        .damage = 81.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 178.2, .poison_damage = { .damage = 26.9973, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 809.992,
        .damage = 243.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 534.6, .poison_damage = { .damage = 80.9919, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 2429.98,
        .damage = 729.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 925.954, .poison_damage = { .damage = 242.976, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 7289.93,
        .damage = 2187.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 1603.8, .poison_damage = { .damage = 728.927, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 21869.8,
        .damage = 6561.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 2777.86, .poison_damage = { .damage = 2186.78, .time = 2.5 } }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 27.0,
        .damage = 45.0009,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 81.0,
        .damage = 135.003,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 243.0,
        .damage = 405.008,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 729.0,
        .damage = 1215.02,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 2187.0,
        .damage = 3645.07,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 6561.0,
        .damage = 10935.2,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 240.003,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.5375,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 720.009,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 2.2969,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 2160.03,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 2.0562,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 6480.08,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.8156,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 19440.2,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.575,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 45.0009,
        .damage = 29.9997,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 216.0, .splash_damage = 216.0, .splash_radius = 1485.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 135.003,
        .damage = 89.9991,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 648.0, .splash_damage = 648.0, .splash_radius = 4455.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 405.008,
        .damage = 269.997,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 1944.0, .splash_damage = 1944.0, .splash_radius = 13365.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 1215.02,
        .damage = 809.992,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 3367.11, .splash_damage = 5832.0, .splash_radius = 26730.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 3645.07,
        .damage = 2429.98,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 5832.0, .splash_damage = 17496.0, .splash_radius = 53460.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 10935.2,
        .damage = 7289.93,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 10101.3, .splash_damage = 52488.0, .splash_radius = 106920.0, .defend_only = 1 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 105.0,
        .damage = 65.9988,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 2.7, .move_speed_factor = 1.2488f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 315.001,
        .damage = 197.996,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 3.025, .move_speed_factor = 1.2834f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 945.003,
        .damage = 593.989,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 3.35, .move_speed_factor = 1.3181f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 2835.01,
        .damage = 1781.97,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 3.675, .move_speed_factor = 1.3528f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 8505.02,
        .damage = 5345.9,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 4.0, .move_speed_factor = 1.3875f }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 5399.99,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.385, .constant_heal = 269.997, .aura_heal = 486.0, .aura_radius = 220.0, .aura_duration = 17.6, .defend_only = 1 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 16200.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.42, .constant_heal = 467.649, .aura_heal = 841.777, .aura_radius = 240.0, .aura_duration = 19.2, .defend_only = 1 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 48600.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.455, .constant_heal = 809.992, .aura_heal = 1458.0, .aura_radius = 260.0, .aura_duration = 20.8, .defend_only = 1 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 145800.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.5075, .constant_heal = 1402.95, .aura_heal = 2525.33, .aura_radius = 290.0, .aura_duration = 23.2, .defend_only = 1 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 59.9967,
        .damage = 59.9967,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.525 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 179.99,
        .damage = 179.99,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.6188 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 539.97,
        .damage = 539.97,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.7125 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 1619.91,
        .damage = 1619.91,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.8063 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 4859.73,
        .damage = 4859.73,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.9 }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 29.9997,
        .damage = 15.0003,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .constant_heal = 4.5,
            .icon_angle = -1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 89.9991,
        .damage = 45.0009,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .constant_heal = 13.5,
            .icon_angle = -1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 269.997,
        .damage = 135.003,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .constant_heal = 40.5,
            .icon_angle = -1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 809.992,
        .damage = 405.008,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .constant_heal = 121.5,
            .icon_angle = -1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 2429.98,
        .damage = 1215.02,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .constant_heal = 210.444,
            .icon_angle = -1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 7289.93,
        .damage = 3645.07,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .constant_heal = 364.5,
            .icon_angle = -1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 21869.8,
        .damage = 10935.2,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .constant_heal = 631.333,
            .icon_angle = -1
        }
    },
    // ---- Full rarity ladders (gen_petal_ladders.py) ----
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 54.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 162.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 1458.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 4374.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 13122.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 39366.0,
        .damage = 26244.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 118098.0,
        .damage = 78732.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 17.7778,
        .damage = 10.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 1.54,
            .slow_inflict_seconds = 1.12,
            .extra_range = 35.0,
            .radial_pulse_damage = 6.1111,
            .radial_pulse_radius = 126.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 53.3334,
        .damage = 30.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 1.76,
            .slow_inflict_seconds = 1.28,
            .extra_range = 40.0,
            .radial_pulse_damage = 18.3333,
            .radial_pulse_radius = 144.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 160.0,
        .damage = 90.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 1.98,
            .slow_inflict_seconds = 1.44,
            .extra_range = 45.0,
            .radial_pulse_damage = 54.9999,
            .radial_pulse_radius = 162.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 480.001,
        .damage = 270.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.09,
            .slow_inflict_seconds = 1.52,
            .extra_range = 47.5,
            .radial_pulse_damage = 165.0,
            .radial_pulse_radius = 171.0
        }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 2.6667,
        .damage = 1.6667,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .move_speed_factor = 1.12f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 6.0,
        .damage = 2.6667,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .poison_damage = { .damage = 2.3333, .time = 1.5 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 5.3333,
        .damage = 3.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .poison_damage = { .damage = 1.0, .time = 4.0 } }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 3.3333,
        .damage = 5.3333,
        .radius = 13.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 10
        }
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 84.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 252.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 756.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 6804.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 20412.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 61236.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 183708.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 5.0,
        .damage = 1.6667,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 15.0,
        .damage = 5.0001,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 2.3281,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 45.0,
        .damage = 15.0003,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 2.1562,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 1.6667,
        .damage = 0.3704,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 0.1852, .on_hit_heal = 1.1 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 5.0001,
        .damage = 1.1112,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 0.5556, .on_hit_heal = 3.3 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 15.0003,
        .damage = 3.3336,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 1.6668, .on_hit_heal = 9.9 }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 10.0,
        .damage = 6.3333,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 1.2346,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 3.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 3.7038,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 2.7938,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 11.1114,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 2.5875,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 33.3342,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.3813,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 0.0617,
        .damage = 0.037,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kCommon,
        .attributes = { .vision_factor = 0.85f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 0.1851,
        .damage = 0.111,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kUnusual,
        .attributes = { .vision_factor = 0.7937f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 0.5553,
        .damage = 0.333,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kRare,
        .attributes = { .vision_factor = 0.7375f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 1.6659,
        .damage = 0.999,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kEpic,
        .attributes = { .vision_factor = 0.6813f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 6.0,
        .damage = 6.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .poison_damage = { .damage = 1.0, .time = 2.0 } }
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 54.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 162.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 1458.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 4374.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 13122.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 39366.0,
        .damage = 26244.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 118098.0,
        .damage = 78732.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 10.0,
        .damage = 14.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 10.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 30.0,
        .damage = 42.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 30.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 90.0,
        .damage = 126.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 90.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "Sturdy. Loses half of its armor each hit.",
        .health = 270.0,
        .damage = 378.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 270.0, .icon_angle = 1.0 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 3.8889,
        .damage = 1.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 0.2778, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 11.6667,
        .damage = 3.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 0.8334, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 1.6667,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 0.55,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.55, .defend_only = 1 }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 20.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { 
            .extra_health = 20.0, .damage_reflection = 0.1f }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 60.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .extra_health = 60.0, .damage_reflection = 0.135f }
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 10.0,
        .damage = 7.7778,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .slow_inflict_seconds = 1.26, .poison_damage = { .damage = 3.1111, .time = 2.8 } }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 30.0,
        .damage = 23.3334,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 1.44, .poison_damage = { .damage = 9.3333, .time = 2.975 } }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 90.0,
        .damage = 70.0002,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 1.62, .poison_damage = { .damage = 27.9999, .time = 3.15 } }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 270.0,
        .damage = 210.001,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 1.71, .poison_damage = { .damage = 83.9997, .time = 3.325 } }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 7.3333,
        .damage = 3.6667,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 17.7778,
        .damage = 4.4444,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 0.245,
            .slow_inflict_seconds = 2.24,
            .aura_radius = 112.0,
            .aura_duration = 8.4,
            .aura_slow_seconds = 1.54,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 53.3334,
        .damage = 13.3332,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 0.28,
            .slow_inflict_seconds = 2.56,
            .aura_radius = 128.0,
            .aura_duration = 9.6,
            .aura_slow_seconds = 1.76,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 160.0,
        .damage = 39.9996,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.315,
            .slow_inflict_seconds = 2.88,
            .aura_radius = 144.0,
            .aura_duration = 10.8,
            .aura_slow_seconds = 1.98,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 480.001,
        .damage = 119.999,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.3325,
            .slow_inflict_seconds = 3.04,
            .aura_radius = 152.0,
            .aura_duration = 11.4,
            .aura_slow_seconds = 2.09,
            .defend_only = 1
        }
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 1.8519,
        .damage = 0.2593,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .move_speed_factor = 1.08f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 5.5557,
        .damage = 0.7779,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .move_speed_factor = 1.1138f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 16.6671,
        .damage = 2.3337,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .move_speed_factor = 1.1475f }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 6.6667,
        .damage = 2.6667,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 1.0,
            .constant_heal = 0.0494,
            .burst_heal = 6.6,
            .extra_rotation_speed = 0.24
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 20.0001,
        .damage = 8.0001,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 0.9313,
            .constant_heal = 0.1482,
            .burst_heal = 19.8,
            .extra_rotation_speed = 0.36
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 60.0003,
        .damage = 24.0003,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.8625,
            .constant_heal = 0.4446,
            .burst_heal = 59.4,
            .extra_rotation_speed = 0.48
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 180.001,
        .damage = 72.0009,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.7937,
            .constant_heal = 1.3338,
            .burst_heal = 178.2,
            .extra_rotation_speed = 0.6
        }
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 1.4815,
        .damage = 0.4444,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 20.0, .poison_armor = 3.3 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 4.4445,
        .damage = 1.3332,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 20.0, .poison_armor = 9.9 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 13.3335,
        .damage = 3.9996,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 20.0, .poison_armor = 29.7 }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 16.6667,
        .damage = 1.6667,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 50.0001,
        .damage = 5.0001,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 150.0,
        .damage = 15.0003,
        .radius = 16.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .icon_angle = 0.5
        }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 1.4815,
        .damage = 0.8519,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .poison_damage = { .damage = 0.2222, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 4.4445,
        .damage = 2.5557,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 0.6666, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 13.3335,
        .damage = 7.6671,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 1.9998, .time = 5.0004 } }
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 54.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 162.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 1458.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 4374.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 13122.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 39366.0,
        .damage = 26244.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 118098.0,
        .damage = 78732.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 21.6667,
        .damage = 3.3333,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_health = 12.0, .damage_reflection = 0.2f }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 4.0,
        .damage = 2.6667,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 12.0,
        .damage = 8.0001,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 36.0,
        .damage = 24.0003,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 108.0,
        .damage = 72.0009,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 324.0,
        .damage = 216.003,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 972.0,
        .damage = 648.008,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 2916.0,
        .damage = 1944.02,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Crown Fragment",
        .description = "A shard of the fallen crown (Boss drop)",
        .health = 8748.0,
        .damage = 5832.07,
        .radius = 15.0,
        .reload = 5.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 12
        }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 0.5556,
        .damage = 0.9259,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_body_damage = 12.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 1.6668,
        .damage = 2.7777,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_body_damage = 36.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 5.0004,
        .damage = 8.3331,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_body_damage = 108.0, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 5.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .dandelion_inflict_seconds = 7.0,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 15.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .dandelion_inflict_seconds = 8.625,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 3.1481,
        .damage = 1.1111,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_damage_factor = 1.18f, .bonus_vs_slowed = 0.25f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 9.4443,
        .damage = 3.3333,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_damage_factor = 1.18f, .bonus_vs_slowed = 0.2875f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 28.3329,
        .damage = 9.9999,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_damage_factor = 1.18f, .bonus_vs_slowed = 0.325f }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 5.9259,
        .damage = 0.1852,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 0.0556, .extra_health = 22.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 17.7777,
        .damage = 0.5556,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 0.1668, .extra_health = 66.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 53.3331,
        .damage = 1.6668,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 0.5004, .extra_health = 198.0 }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 20.0,
        .damage = 1.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { 
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 60.0,
        .damage = 3.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .secondary_reload = 2.3281,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 180.0,
        .damage = 9.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { 
            .secondary_reload = 2.1562,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kBeetle
        }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 13.3333,
        .damage = 1.6667,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 2.75 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 5.0,
        .damage = 12.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_rotation_speed = 0.45 }
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 6.6667,
        .damage = 7.3333,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .burn_damage = { .damage = 1.3333, .time = 3.0 } }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 2.2222,
        .damage = 1.2593,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_rotation_speed = 0.15, .extra_range = 0.2963 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 6.6666,
        .damage = 3.7779,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_rotation_speed = 0.225, .extra_range = 0.8889 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 19.9998,
        .damage = 11.3337,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.3, .extra_range = 2.6667 }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 2.037,
        .damage = 1.7778,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_damage_factor = 1.14f, .armor_penetration = 0.25f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 6.111,
        .damage = 5.3334,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_damage_factor = 1.14f, .armor_penetration = 0.2875f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 18.333,
        .damage = 16.0002,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_damage_factor = 1.14f, .armor_penetration = 0.325f }
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 0.9259,
        .damage = 0.8889,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 8.0, .slow_inflict_seconds = 1.4, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 2.7777,
        .damage = 2.6667,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 24.0, .slow_inflict_seconds = 1.725, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 8.3331,
        .damage = 8.0001,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 72.0, .slow_inflict_seconds = 2.05, .defend_only = 1 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 5.5556,
        .damage = 0.1852,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .poison_armor = 4.4, .extra_health = 32.0 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 16.6668,
        .damage = 0.5556,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_armor = 13.2, .extra_health = 96.0 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 50.0004,
        .damage = 1.6668,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_armor = 39.6, .extra_health = 288.0 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 1.8519,
        .damage = 1.5556,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .slow_inflict_seconds = 1.4, .extra_health = 12.0 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 5.5557,
        .damage = 4.6668,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 1.725, .extra_health = 36.0 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 16.6671,
        .damage = 14.0004,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 2.05, .extra_health = 108.0 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 1.6667,
        .damage = 0.963,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 5.0001,
        .damage = 2.889,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 15.0003,
        .damage = 8.667,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 1.2963,
        .damage = 0.5926,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 3.8889,
        .damage = 1.7778,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.4656,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 11.6667,
        .damage = 5.3334,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.4313,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 1.6667,
        .damage = 1.1111,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 1.0, .extra_reload_factor = 0.97f, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns (stacks; hard cap 99% CDR).",
        .health = 5.0001,
        .damage = 3.3333,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 3.0, .extra_reload_factor = 0.9512f, .icon_angle = -1.0 }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 11.6667,
        .damage = 0.3333,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 35.0001,
        .damage = 0.9999,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 2.3281,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 105.0,
        .damage = 2.9997,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 2.1562,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 6.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 2.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 46.6667,
        .damage = 1.3333,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 2.4444, .extra_health = 57.7778, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 140.0,
        .damage = 3.9999,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 7.3332, .extra_health = 173.333, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 420.0,
        .damage = 11.9997,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 21.9996, .extra_health = 520.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 1260.0,
        .damage = 35.9991,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 65.9988, .extra_health = 1560.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 13.3333,
        .damage = 4.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 39.9999,
        .damage = 12.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 42.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 120.0,
        .damage = 36.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 126.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 359.999,
        .damage = 108.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 378.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 1080.0,
        .damage = 324.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 1134.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 3239.99,
        .damage = 972.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 3402.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 9719.98,
        .damage = 2916.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 5892.44, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 29159.9,
        .damage = 8748.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 10206.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 84.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 252.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 756.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 6804.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 20412.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 61236.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 183708.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 2.4691,
        .damage = 0.1235,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 7.4073,
        .damage = 0.3705,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 22.2219,
        .damage = 1.1115,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 5.1852,
        .damage = 1.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 3.3, .extra_rotation_speed = 0.15 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 15.5556,
        .damage = 3.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 9.9, .extra_rotation_speed = 0.225 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 46.6668,
        .damage = 9.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 29.7, .extra_rotation_speed = 0.3 }
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 84.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 252.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 756.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 6804.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 20412.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 61236.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 183708.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 2.5926,
        .damage = 0.9259,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 36.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 7.7778,
        .damage = 2.7777,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.9313, .burst_heal = 108.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 23.3334,
        .damage = 8.3331,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.8625, .burst_heal = 324.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 8.0,
        .damage = 6.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 20.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 24.0,
        .damage = 18.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.9313, .burst_heal = 60.0, .defend_only = 1 }
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 1.4815,
        .damage = 0.1852,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 1.0, .constant_heal = 0.1111, .burst_heal = 16.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 4.4445,
        .damage = 0.5556,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.9313, .constant_heal = 0.3333, .burst_heal = 48.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 13.3335,
        .damage = 1.6668,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.8625, .constant_heal = 0.9999, .burst_heal = 144.0, .defend_only = 1 }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 1.1111,
        .damage = 0.1481,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .constant_heal = 0.0741,
            .poison_damage = { .damage = 0.1111, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 3.3333,
        .damage = 0.4443,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .constant_heal = 0.2223,
            .poison_damage = { .damage = 0.3333, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 9.9999,
        .damage = 1.3329,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .constant_heal = 0.6669,
            .poison_damage = { .damage = 0.9999, .time = 2.9997 }
        }
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 12.2222,
        .damage = 0.6667,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 10.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 36.6666,
        .damage = 2.0001,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 30.0, .defend_only = 1 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 7.3333,
        .damage = 6.6667,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .slow_inflict_seconds = 1.05 }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 1.8519,
        .damage = 0.6667,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 3.0,
            .slow_inflict_seconds = 0.315,
            .extra_range = 0.9259,
            .radial_pulse_damage = 0.6667,
            .radial_pulse_radius = 4.0741
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 5.5557,
        .damage = 2.0001,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 2.7938,
            .slow_inflict_seconds = 0.3881,
            .extra_range = 2.7777,
            .radial_pulse_damage = 2.0001,
            .radial_pulse_radius = 12.2223
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 16.6671,
        .damage = 6.0003,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 2.5875,
            .slow_inflict_seconds = 0.4612,
            .extra_range = 8.3331,
            .radial_pulse_damage = 6.0003,
            .radial_pulse_radius = 36.6669
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 31.1111,
        .damage = 10.5556,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .extra_health = 266.667,
            .burn_damage = { .damage = 2.0, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 93.3333,
        .damage = 31.6668,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_health = 800.0,
            .burn_damage = { .damage = 6.0, .time = 4.25 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 280.0,
        .damage = 95.0004,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_health = 2400.0,
            .burn_damage = { .damage = 18.0, .time = 4.5 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 840.0,
        .damage = 285.001,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_health = 7200.0,
            .burn_damage = { .damage = 54.0, .time = 4.75 }
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
        .rarity = RarityID::kCommon,
        .attributes = { 
            .poison_damage = {
                .damage = 3.3333,
                .time = 2.0
            }
        }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 4.4444,
        .damage = 2.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_damage_factor = 1.15f }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 13.3332,
        .damage = 6.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_damage_factor = 1.1787f }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 100.0,
        .damage = 66.6667,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .extra_damage_factor = 1.3300f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 300.0,
        .damage = 200.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_damage_factor = 1.3700f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 900.0,
        .damage = 600.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_damage_factor = 1.4100f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 2700.0,
        .damage = 1800.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_damage_factor = 1.4500f
        }
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
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
        .rarity = RarityID::kCommon,
        .attributes = {
            .constant_heal = 1.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 84.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 252.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 756.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 6804.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 20412.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 61236.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 183708.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 1.8519,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 0.1481, .extra_health = 15.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 5.5557,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 0.4443, .extra_health = 45.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 16.6671,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 1.3329, .extra_health = 135.0, .defend_only = 1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 0.2469,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 1.0, .poison_armor = 6.0, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 0.7407,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 3.0, .poison_armor = 18.0, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 2.2221,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 9.0, .poison_armor = 54.0, .icon_angle = 0.1 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 25.0,
        .damage = 9.3333,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_health = 10.0, .burn_damage = { .damage = 0.6667, .time = 3.0 }, .splash_damage = 12.0, .splash_radius = 70.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 75.0,
        .damage = 27.9999,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 30.0, .burn_damage = { .damage = 2.0001, .time = 3.0 }, .splash_damage = 36.0, .splash_radius = 210.0 }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 3.3333,
        .damage = 1.6667,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .magnet_radius = 500.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 1.0,
        .damage = 1.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 3.0,
        .damage = 3.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 9.0,
        .damage = 9.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 26.6667,
        .damage = 6.6667,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_rotation_speed = 0.108 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 80.0001,
        .damage = 20.0001,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_rotation_speed = 0.162 }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 1.2963,
        .damage = 0.5926,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = {
            .extra_rotation_speed = 0.084,
            .poison_damage = { .damage = 0.1111, .time = 2.0007 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 3.8889,
        .damage = 1.7778,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_rotation_speed = 0.126,
            .poison_damage = { .damage = 0.3333, .time = 2.0007 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 11.6667,
        .damage = 5.3334,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_rotation_speed = 0.168,
            .poison_damage = { .damage = 0.9999, .time = 2.0007 }
        }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 1.3333,
        .damage = 4.4333,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 1.4815,
        .damage = 1.4815,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 4.4445,
        .damage = 4.4445,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .burn_damage = { .damage = 18.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 13.3335,
        .damage = 13.3335,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .burn_damage = { .damage = 54.0, .time = 3.0 } }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 0.3704,
        .damage = 0.0617,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.7, .aura_heal = 4.0, .aura_radius = 140.0, .aura_duration = 12.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 1.1112,
        .damage = 0.1851,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.6562, .aura_heal = 12.0, .aura_radius = 175.0, .aura_duration = 14.0, .defend_only = 1, .icon_angle = -0.35f }
    },
    // Moon ladder: high HP; mass ≈ gardn-master density=10 scale. Detached orbit center.
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 500.0,
        .damage = 0.3,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mass = 4.0 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 1500.0,
        .damage = 0.9,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mass = 5.0 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 4500.0,
        .damage = 2.7,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 6.0 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 13500.0,
        .damage = 8.1,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mass = 7.0 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 40500.0,
        .damage = 24.3,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mass = 8.0 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 121500.0,
        .damage = 72.9,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mass = 9.0 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 364500.0,
        .damage = 218.7,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mass = 9.5 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 1093500.0,
        .damage = 656.1,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mass = 9.75 }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 0.0494,
        .damage = 0.0206,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kCommon,
        .attributes = { .vision_factor = 0.55f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 0.1482,
        .damage = 0.0618,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kUnusual,
        .attributes = { .vision_factor = 0.5038f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 0.4446,
        .damage = 0.1854,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kRare,
        .attributes = { .vision_factor = 0.4575f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 1.3338,
        .damage = 0.5562,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kEpic,
        .attributes = { .vision_factor = 0.4113f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 4.0014,
        .damage = 1.6686,
        .radius = 12.5,
        .reload = 2.5,
        .count = 0,
        .rarity = RarityID::kLegendary,
        .attributes = { .vision_factor = 0.365f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 8.0,
        .damage = 11.3333,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 24.0,
        .damage = 33.9999,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor_penetration = 0.5312f }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 8.8889,
        .damage = 0.2963,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .damage_reflection = 0.1f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 26.6667,
        .damage = 0.8889,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .damage_reflection = 0.135f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 80.0001,
        .damage = 2.6667,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .damage_reflection = 0.17f, .defend_only = 1 }
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal",
        .health = 5.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal",
        .health = 15.0,
        .damage = 24.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 8.0,
            .secondary_reload = 0.4656,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 2.5926,
        .damage = 1.037,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 4.0, .extra_range = 1.1111 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 7.7778,
        .damage = 3.111,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 12.0, .extra_range = 3.3333 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 23.3334,
        .damage = 9.333,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 36.0, .extra_range = 9.9999 }
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 84.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 252.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 756.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 6804.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 20412.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 61236.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 183708.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 1.0,
        .damage = 1.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.12f
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 3.0,
        .damage = 4.5,
        .radius = 12.0,
        .reload = 2.25,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 9,
            .extra_damage_factor = 1.148f
        }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 3.1111,
        .damage = 1.1111,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_rotation_speed = 0.192 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 9.3333,
        .damage = 3.3333,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_rotation_speed = 0.288 }
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 54.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 162.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 1458.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 4374.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 13122.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 39366.0,
        .damage = 26244.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 118098.0,
        .damage = 78732.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 1.1111,
        .damage = 1.1111,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .slow_inflict_seconds = 1.4,
            .poison_damage = {
                .damage = 0.5556,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 3.3333,
        .damage = 3.3333,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .slow_inflict_seconds = 1.725,
            .poison_damage = {
                .damage = 1.6668,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 2.037,
        .damage = 0.2963,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 3.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 6.111,
        .damage = 0.8889,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 2.7938,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 18.333,
        .damage = 2.6667,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 2.5875,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 12.0,
        .damage = 4.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 4.0, .time = 4.0 } }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 5.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 0.5,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 15.0,
        .damage = 24.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 0.4656,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 45.0,
        .damage = 72.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.4313,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 1.4815,
        .damage = 0.2963,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.5, .slow_inflict_seconds = 2.1, .aura_heal = 0.1111, .aura_radius = 3.5185, .aura_duration = 2.2, .defend_only = 1 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 4.4445,
        .damage = 0.8889,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.4656, .slow_inflict_seconds = 2.5875, .aura_heal = 0.3333, .aura_radius = 10.5555, .aura_duration = 3.325, .defend_only = 1 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 13.3335,
        .damage = 2.6667,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.4313, .slow_inflict_seconds = 3.075, .aura_heal = 0.9999, .aura_radius = 31.6665, .aura_duration = 4.45, .defend_only = 1 }
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 54.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 162.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 1458.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 4374.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 13122.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 39366.0,
        .damage = 26244.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 118098.0,
        .damage = 78732.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 8.3333,
        .damage = 1.6667,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 5.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 24.9999,
        .damage = 5.0001,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 4.6562,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 74.9997,
        .damage = 15.0003,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 4.3125,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 224.999,
        .damage = 45.0009,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 3.9688,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 2.4444,
        .damage = 0.8889,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .slow_inflict_seconds = 1.4 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 7.3332,
        .damage = 2.6667,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 1.725 }
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 1.6667,
        .damage = 2.6667,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 5.0001,
        .damage = 8.0001,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 15.0003,
        .damage = 24.0003,
        .radius = 13.0,
        .reload = 0.12,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .icon_angle = 0.7
        }
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 50.0,
        .damage = 5.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kCommon,
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
        .rarity = RarityID::kCommon,
        .attributes = { 
            .secondary_reload = 1.0,
            .burst_heal = 14.0,
            .defend_only = 1
        }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 13.3333,
        .damage = 0.8889,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .poison_damage = { .damage = 0.6667, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 39.9999,
        .damage = 2.6667,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 2.0001, .time = 2.0 } }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 4.6667,
        .damage = 1.8333,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .extra_rotation_speed = 0.21
        }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 6.6667,
        .damage = 1.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 5.5, .extra_health = 13.75 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 20.0001,
        .damage = 3.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 16.5, .extra_health = 41.25 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 60.0003,
        .damage = 9.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 49.5, .extra_health = 123.75 }
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 54.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 162.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 1458.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 4374.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 13122.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 39366.0,
        .damage = 26244.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 118098.0,
        .damage = 78732.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 1.0,
        .damage = 1.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 0.8889, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 3.0,
        .damage = 3.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 2.6667, .time = 3.0 }
        }
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 54.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 162.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 1458.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 4374.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 13122.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 39366.0,
        .damage = 26244.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 118098.0,
        .damage = 78732.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 1.0,
        .damage = 2.2222,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .extra_range = 40.0f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 3.0,
        .damage = 6.6666,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_range = 70.0f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 2.037,
        .damage = 1.3333,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .extra_damage_factor = 1.22f,
            .execute_threshold = 0.25f,
            .execute_factor = 1.6f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 6.111,
        .damage = 3.9999,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_damage_factor = 1.22f,
            .execute_threshold = 0.275f,
            .execute_factor = 1.67f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 18.333,
        .damage = 11.9997,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_damage_factor = 1.22f,
            .execute_threshold = 0.3f,
            .execute_factor = 1.74f
        }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 1.1111,
        .damage = 1.3333,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .damage_reflection = 0.12f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 3.3333,
        .damage = 3.9999,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .damage_reflection = 0.155f }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 1.1111,
        .damage = 1.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 3.3333,
        .damage = 3.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 10,
        }
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 0.7407,
        .damage = 0.6296,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 2.0,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 2.2221,
        .damage = 1.8888,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 1.8625,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 6.6663,
        .damage = 5.6664,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 1.725,
            .defend_only = 1,
            .icon_angle = 1.0,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 10.6667,
        .damage = 5.3333,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 18.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 54.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 162.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 1458.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 4374.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 13122.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 39366.0,
        .damage = 26244.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 118098.0,
        .damage = 78732.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 84.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 252.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 756.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 6804.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 20412.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 61236.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 183708.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 4.8148,
        .damage = 0.3704,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .damage_reflection = 0.15f,
            .burn_damage = { .damage = 5.0, .time = 1.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 14.4444,
        .damage = 1.1112,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .damage_reflection = 0.185f,
            .burn_damage = { .damage = 15.0, .time = 1.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 43.3332,
        .damage = 3.3336,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .damage_reflection = 0.22f,
            .burn_damage = { .damage = 45.0, .time = 1.0 }
        }
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 17.3333,
        .damage = 2.6667,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 2.2 }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 13.3333,
        .damage = 15.5556,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_damage_factor = 1.4300f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 39.9999,
        .damage = 46.6668,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_damage_factor = 1.4700f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 120.0,
        .damage = 140.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_damage_factor = 1.5100f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 359.999,
        .damage = 420.001,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.5500f }
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 15.0,
        .damage = 3.6667,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 4.4444,
        .damage = 0.1111,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 13.3332,
        .damage = 0.3333,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 2.3281,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 39.9996,
        .damage = 0.9999,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 2.1562,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
        }
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 84.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 252.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 756.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 6804.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 20412.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 61236.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 183708.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 1.0,
        .damage = 1.8889,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 3.0,
        .damage = 5.6667,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 4.0,
        .damage = 1.6667,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .poison_damage = { .damage = 0.2222, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 12.0,
        .damage = 5.0001,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 0.6666, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 36.0,
        .damage = 15.0003,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 1.9998, .time = 3.0 } }
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 52488.0,
        .damage = 32805.0,
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
        .damage = 3.3333,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 3.5,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 30.0,
        .damage = 9.9999,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 3.3,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 90.0,
        .damage = 29.9997,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 3.1,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 270.0,
        .damage = 89.9991,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.9,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 810.0,
        .damage = 269.997,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.7,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 2430.0,
        .damage = 809.992,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 7290.0,
        .damage = 2429.98,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 2.3,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 21870.0,
        .damage = 7289.93,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 2.1,
            .defend_only = 1,
            .icon_angle = M_PI / 4 + 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSquare
        }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 4.4444,
        .damage = 1.6667,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.75f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 13.3332,
        .damage = 5.0001,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.8266f }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 16.6667,
        .damage = 10.6667,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_health = 6.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 50.0001,
        .damage = 32.0001,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 18.0 }
    },
    {
        .name = "Stick",
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 10.0,
        .damage = 1.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Stick",
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 30.0,
        .damage = 3.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Stick",
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 90.0,
        .damage = 9.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Stick",
        .description = "A mysterious stick that summons the forces of the wind.",
        .health = 270.0,
        .damage = 27.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 12.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 1
        }
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 3.0,
        .damage = 70.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 8.0,
        .damage = 1.1667,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .armor = 5.5
        }
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 7.037,
        .damage = 0.1852,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 5.0, .poison_armor = 9.9 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 21.111,
        .damage = 0.5556,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 15.0, .poison_armor = 29.7 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 63.333,
        .damage = 1.6668,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 45.0, .poison_armor = 89.1 }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 0.0412,
        .damage = 0.0206,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_range = 25.0, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 0.1236,
        .damage = 0.0618,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_range = 36.875, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 0.3708,
        .damage = 0.1854,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_range = 48.75, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 1.1124,
        .damage = 0.5562,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_range = 60.625, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 3.3372,
        .damage = 1.6686,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_range = 72.5, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 3.3333,
        .damage = 0.8889,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .damage_reflection = 0.15f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 9.9999,
        .damage = 2.6667,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .damage_reflection = 0.185f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 29.9997,
        .damage = 8.0001,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .damage_reflection = 0.22f }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 1.6667,
        .damage = 1.2222,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 5.0001,
        .damage = 3.6666,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 1.1111,
        .damage = 1.2778,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 1.05,
            .poison_damage = { .damage = 0.4444, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 3.3333,
        .damage = 3.8334,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 1.2937,
            .poison_damage = { .damage = 1.3332, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 18.3333,
        .damage = 4.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .slow_inflict_seconds = 0.77, .snare_seconds = 0.56 }
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 28.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 84.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 252.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 756.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 6804.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 20412.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 61236.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 183708.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 3.1111,
        .damage = 1.3333,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 2.5,
            .extra_range = 2.0,
            .radial_pulse_damage = 1.5556,
            .radial_pulse_radius = 10.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 9.3333,
        .damage = 3.9999,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 2.3281,
            .extra_range = 6.0,
            .radial_pulse_damage = 4.6668,
            .radial_pulse_radius = 30.0
        }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 2.0,
        .damage = 1.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 4.4 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 6.0,
        .damage = 3.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 13.2 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 9.6296,
        .damage = 0.2963,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 16.0, .extra_health = 18.0, .taunt_radius = 120.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 28.8888,
        .damage = 0.8889,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 48.0, .extra_health = 54.0, .taunt_radius = 155.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 86.6664,
        .damage = 2.6667,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 144.0, .extra_health = 162.0, .taunt_radius = 190.0 }
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 5.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 15.0,
        .damage = 24.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 45.0,
        .damage = 72.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 10.5556,
        .damage = 1.3333,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mass = 36.0, .armor = 1.65 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 31.6668,
        .damage = 3.9999,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mass = 53.5, .armor = 4.95 }
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 5.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 8.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 72.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 216.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 648.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 1944.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 5832.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 17496.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 52488.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 3.3333,
        .damage = 1.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 2.2, .poison_damage = { .damage = 0.3333, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 9.9999,
        .damage = 3.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 6.6, .poison_damage = { .damage = 0.9999, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 29.9997,
        .damage = 9.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 19.8, .poison_damage = { .damage = 2.9997, .time = 2.5 } }
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 12.0,
        .damage = 8.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 36.0,
        .damage = 24.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 324.0,
        .damage = 216.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 972.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 2916.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 26244.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 78732.0,
        .damage = 52488.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 1.0,
        .damage = 1.6667,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 12, .extra_range = 4.4444f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 3.0,
        .damage = 5.0001,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 12, .extra_range = 13.3332f }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 2.963,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 3.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 8.889,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 3.2594,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 26.667,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 3.0187,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 1.6667,
        .damage = 1.1111,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_health = 8.0, .splash_damage = 8.0, .splash_radius = 55.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 5.0001,
        .damage = 3.3333,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 24.0, .splash_damage = 24.0, .splash_radius = 165.0, .defend_only = 1 }
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 1.6667,
        .damage = 1.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.7, .defend_only = 1 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 1.2963,
        .damage = 0.8148,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .slow_inflict_seconds = 1.4, .move_speed_factor = 1.11f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 3.8889,
        .damage = 2.4444,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 1.725, .move_speed_factor = 1.1447f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 11.6667,
        .damage = 7.3332,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 2.05, .move_speed_factor = 1.1794f }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 15.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 22.2222,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.245, .constant_heal = 1.1111, .aura_heal = 2.0, .aura_radius = 140.0, .aura_duration = 11.2, .defend_only = 1 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 66.6666,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.28, .constant_heal = 3.3333, .aura_heal = 6.0, .aura_radius = 160.0, .aura_duration = 12.8, .defend_only = 1 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 200.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.315, .constant_heal = 9.9999, .aura_heal = 18.0, .aura_radius = 180.0, .aura_duration = 14.4, .defend_only = 1 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 599.999,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.3325, .constant_heal = 29.9997, .aura_heal = 54.0, .aura_radius = 190.0, .aura_duration = 15.2, .defend_only = 1 }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 0.6173,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 1.8519,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 5.5557,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 16.6671,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 50.0013,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 150.004,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 450.012,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 1350.04,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .defend_only = 1, .icon_angle = M_PI }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 0.7407,
        .damage = 0.7407,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_rotation_speed = 0.15 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 2.2221,
        .damage = 2.2221,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_rotation_speed = 0.2437 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 6.6663,
        .damage = 6.6663,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.3375 }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 3.3333,
        .damage = 1.6667,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .constant_heal = 0.5,
            .icon_angle = -1
        }
    },
    // ---- Triangle (Ant Nest / Termite) ----
    // Base damage is 1-copy; tip/combat add stack_damage per extra same-rarity copy.
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 10.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .stack_damage = 4.0 }
    },
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 30.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .stack_damage = 12.0 }
    },
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 90.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .stack_damage = 36.0 }
    },
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 270.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .stack_damage = 108.0 }
    },
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 810.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .stack_damage = 324.0 }
    },
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 2430.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .stack_damage = 972.0 }
    },
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 7290.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .stack_damage = 2916.0 }
    },
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 21870.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .stack_damage = 8748.0 }
    },
    {
        .name = "Triangle",
        .description = "Starts at base damage; each extra same-rarity copy adds stack damage to all of them.",
        .health = 65610.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .stack_damage = 26244.0 }
    },
    // ---- Wiki import wave ----
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 12.0,
        .damage = 13.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .attraction_radius = 35.0 }
    },
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 36.0,
        .damage = 39.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .attraction_radius = 50.0 }
    },
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 108.0,
        .damage = 117.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .attraction_radius = 70.0 }
    },
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 324.0,
        .damage = 351.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .attraction_radius = 95.0 }
    },
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 972.0,
        .damage = 1053.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .attraction_radius = 125.0 }
    },
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 2916.0,
        .damage = 3159.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .attraction_radius = 160.0 }
    },
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 8748.0,
        .damage = 9477.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .attraction_radius = 200.0 }
    },
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 26244.0,
        .damage = 28431.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .attraction_radius = 250.0 }
    },
    {
        .name = "Lentil",
        .description = "Increases petal attraction to enemies.",
        .health = 78732.0,
        .damage = 85293.0,
        .radius = 9.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .attraction_radius = 310.0 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 25.0,
        .damage = 40.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 75.0,
        .damage = 120.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 225.0,
        .damage = 360.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 675.0,
        .damage = 1080.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 2025.0,
        .damage = 3240.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 6075.0,
        .damage = 9720.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 18225.0,
        .damage = 29160.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 54675.0,
        .damage = 87480.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Sawblade",
        .description = "This looks like it'd hurt.",
        .health = 164025.0,
        .damage = 262440.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .fixed_orbit_range = 1 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 30.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 90.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 270.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 810.0,
        .damage = 810.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 2430.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 7290.0,
        .damage = 7290.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 21870.0,
        .damage = 21870.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Dice",
        .description = "Has a small chance of dealing 35x damage. Luck increases the chance.",
        .health = 65610.0,
        .damage = 65610.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .crit_chance = 0.05, .crit_factor = 35.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 16.0,
        .damage = 18.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .domino_max_damage = 36.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 48.0,
        .damage = 54.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .domino_max_damage = 108.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 144.0,
        .damage = 162.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .domino_max_damage = 324.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 432.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .domino_max_damage = 972.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 1296.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .domino_max_damage = 2916.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 3888.0,
        .damage = 4374.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .domino_max_damage = 8748.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 11664.0,
        .damage = 13122.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .domino_max_damage = 26244.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 34992.0,
        .damage = 39366.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .domino_max_damage = 78732.0 }
    },
    {
        .name = "Domino",
        .description = "Damage depends on how many dots it has.",
        .health = 104976.0,
        .damage = 118098.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .domino_max_damage = 236196.0 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 25.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 75.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 225.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 675.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 2025.0,
        .damage = 810.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 6075.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 18225.0,
        .damage = 7290.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 54675.0,
        .damage = 21870.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Card",
        .description = "A playing card. Likely marked.",
        .health = 164025.0,
        .damage = 65610.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .card_random_stats = 1 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 1.0,
        .damage = 7.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 3.0,
        .damage = 21.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 9.0,
        .damage = 63.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 27.0,
        .damage = 189.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 81.0,
        .damage = 567.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 243.0,
        .damage = 1701.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 729.0,
        .damage = 5103.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 2187.0,
        .damage = 15309.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Poker Chip",
        .description = "ALL IN.",
        .health = 6561.0,
        .damage = 45927.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .petal_evasion = 0.90 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 12.0,
        .damage = 20.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 36.0,
        .damage = 60.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 108.0,
        .damage = 180.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 324.0,
        .damage = 540.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 972.0,
        .damage = 1620.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 2916.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 8748.0,
        .damage = 14580.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 26244.0,
        .damage = 43740.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Bean",
        .description = "Deals more damage if there are no allied flowers around you.",
        .health = 78732.0,
        .damage = 131220.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .solo_damage_factor = 1.5 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 10.0,
        .damage = 5.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .clay_max_damage = 100.0 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 30.0,
        .damage = 15.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .clay_max_damage = 300.0 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 90.0,
        .damage = 45.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .clay_max_damage = 900.0 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 270.0,
        .damage = 135.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .clay_max_damage = 2700.0 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 810.0,
        .damage = 405.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .clay_max_damage = 8100.0 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 2430.0,
        .damage = 1215.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .clay_max_damage = 24300.0 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 7290.0,
        .damage = 3645.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .clay_max_damage = 72900.0 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 21870.0,
        .damage = 10935.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .clay_max_damage = 218700.0 }
    },
    {
        .name = "Clay",
        .description = "It sticks to dust in the air to get bigger and stronger.",
        .health = 65610.0,
        .damage = 32805.0,
        .radius = 11.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .clay_max_damage = 656100.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 10.0,
        .damage = 5.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .tomato_max_damage = 70.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 30.0,
        .damage = 15.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .tomato_max_damage = 210.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 90.0,
        .damage = 45.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .tomato_max_damage = 630.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 270.0,
        .damage = 135.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .tomato_max_damage = 1890.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 810.0,
        .damage = 405.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .tomato_max_damage = 5670.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 2430.0,
        .damage = 1215.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .tomato_max_damage = 17010.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 7290.0,
        .damage = 3645.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .tomato_max_damage = 51030.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 21870.0,
        .damage = 10935.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .tomato_max_damage = 153090.0 }
    },
    {
        .name = "Tomato",
        .description = "Gets stronger over time.",
        .health = 65610.0,
        .damage = 32805.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .tomato_max_damage = 459270.0 }
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 1.7,
        .damage = 4.3,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 5.1,
        .damage = 12.9,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 15.3,
        .damage = 38.7,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 45.9,
        .damage = 116.1,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 137.7,
        .damage = 348.3,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 413.1,
        .damage = 1044.9,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 1239.3,
        .damage = 3134.7,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 3717.9,
        .damage = 9404.1,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Dust",
        .description = "Sometimes it sticks together, sometimes it doesn't.",
        .health = 11153.7,
        .damage = 28212.3,
        .radius = 6.0,
        .reload = 0.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 15.0,
        .damage = 10.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .splash_damage = 5.0, .splash_radius = 40.0 }
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 45.0,
        .damage = 30.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .splash_damage = 15.0, .splash_radius = 40.0 }
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 135.0,
        .damage = 90.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .splash_damage = 45.0, .splash_radius = 40.0 }
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 405.0,
        .damage = 270.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .splash_damage = 135.0, .splash_radius = 40.0 }
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 1215.0,
        .damage = 810.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .splash_damage = 405.0, .splash_radius = 40.0 }
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 3645.0,
        .damage = 2430.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .splash_damage = 1215.0, .splash_radius = 40.0 }
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 10935.0,
        .damage = 7290.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .splash_damage = 3645.0, .splash_radius = 40.0 }
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 32805.0,
        .damage = 21870.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .splash_damage = 10935.0, .splash_radius = 40.0 }
    },
    {
        .name = "Broccoli",
        .description = "Explodes when it hits something.",
        .health = 98415.0,
        .damage = 65610.0,
        .radius = 11.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .splash_damage = 32805.0, .splash_radius = 40.0 }
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 6.7,
        .damage = 8.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 20.1,
        .damage = 24.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 60.3,
        .damage = 72.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 180.9,
        .damage = 216.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 542.7,
        .damage = 648.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 1628.1,
        .damage = 1944.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 4884.3,
        .damage = 5832.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 14652.9,
        .damage = 17496.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Orange",
        .description = "A sweet citrus fruit.",
        .health = 43958.7,
        .damage = 52488.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 15.0,
        .damage = 18.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 45.0,
        .damage = 54.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 135.0,
        .damage = 162.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 405.0,
        .damage = 486.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 1215.0,
        .damage = 1458.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 3645.0,
        .damage = 4374.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 10935.0,
        .damage = 13122.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 32805.0,
        .damage = 39366.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Plank",
        .description = "Does 20x damage vs projectiles and petals.",
        .health = 98415.0,
        .damage = 118098.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .vs_projectile_factor = 20.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 5.0,
        .damage = 5.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor_shred = 1.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 15.0,
        .damage = 15.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor_shred = 4.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 45.0,
        .damage = 45.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor_shred = 13.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 135.0,
        .damage = 135.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor_shred = 40.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 405.0,
        .damage = 405.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor_shred = 121.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 1215.0,
        .damage = 1215.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor_shred = 364.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 3645.0,
        .damage = 3645.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor_shred = 1093.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 10935.0,
        .damage = 10935.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor_shred = 3280.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Bur",
        .description = "Decreases armor of affected unit. Does not stack.",
        .health = 32805.0,
        .damage = 32805.0,
        .radius = 8.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor_shred = 9841.5, .armor_shred_seconds = 3.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 10.0,
        .damage = 10.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_health = 50.0, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 30.0,
        .damage = 30.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 150.0, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 90.0,
        .damage = 90.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 450.0, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 270.0,
        .damage = 270.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 1350.0, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 810.0,
        .damage = 810.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 4050.0, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 2430.0,
        .damage = 2430.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 12150.0, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 7290.0,
        .damage = 7290.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 21044.4, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 21870.0,
        .damage = 21870.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 36450.0, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Soil",
        .description = "Increases health, but also size and decreases move speed.",
        .health = 65610.0,
        .damage = 65610.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 63133.3, .move_speed_factor = 0.92, .body_radius_bonus = 10.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .body_radius_bonus = 8.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .body_radius_bonus = 16.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .body_radius_bonus = 24.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .body_radius_bonus = 32.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .body_radius_bonus = 40.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .body_radius_bonus = 48.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .body_radius_bonus = 56.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .body_radius_bonus = 64.0 }
    },
    {
        .name = "Air",
        .description = "It's literally nothing.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 1.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .body_radius_bonus = 72.0 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .heal_amp = 0.2 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .heal_amp = 0.25 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .heal_amp = 0.3 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .heal_amp = 0.35 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .heal_amp = 0.4 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .heal_amp = 0.45 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .heal_amp = 0.5 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .heal_amp = 0.55 }
    },
    {
        .name = "Basil",
        .description = "Increases all healing received.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .heal_amp = 0.6 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 2.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .damage_absorb = 2.0 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 6.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .damage_absorb = 6.0 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 18.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .damage_absorb = 18.0 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 54.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_absorb = 54.0 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 162.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_absorb = 162.0 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 486.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_absorb = 486.0 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 1458.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_absorb = 841.777 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 4374.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_absorb = 1458.0 }
    },
    {
        .name = "Cotton",
        .description = "Absorbs damage taken by the flower.",
        .health = 13122.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_absorb = 2525.33 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 30.0,
        .damage = 1.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .knockback_taken = 0.3 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 90.0,
        .damage = 3.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .knockback_taken = 0.6 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 270.0,
        .damage = 9.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .knockback_taken = 0.9 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 810.0,
        .damage = 27.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .knockback_taken = 1.2 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 2430.0,
        .damage = 81.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .knockback_taken = 1.5 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 7290.0,
        .damage = 243.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .knockback_taken = 1.8 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 21870.0,
        .damage = 729.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .knockback_taken = 2.1 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 65610.0,
        .damage = 2187.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .knockback_taken = 2.4 }
    },
    {
        .name = "Rubber",
        .description = "Increases knockback received. Absorbs lightning damage.",
        .health = 196830.0,
        .damage = 6561.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .knockback_taken = 2.7 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 18.0,
        .damage = 18.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 54.0,
        .damage = 54.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 162.0,
        .damage = 162.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 486.0,
        .damage = 486.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 1458.0,
        .damage = 1458.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 4374.0,
        .damage = 4374.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 13122.0,
        .damage = 13122.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 39366.0,
        .damage = 39366.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Carrot",
        .description = "Sturdy and reliable. Can be shot and bounces off walls.",
        .health = 118098.0,
        .damage = 118098.0,
        .radius = 9.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 5.0,
        .damage = 3.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kCommon,
        .attributes = { .poison_damage = { .damage = 25.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 15.0,
        .damage = 9.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 75.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 45.0,
        .damage = 27.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 225.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 135.0,
        .damage = 81.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 675.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 405.0,
        .damage = 243.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 2025.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 1215.0,
        .damage = 729.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 6075.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 3645.0,
        .damage = 2187.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 18225.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 10935.0,
        .damage = 6561.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 54675.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Grapes",
        .description = "It goes poof. Now with poison.",
        .health = 32805.0,
        .damage = 19683.0,
        .radius = 8.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 164025.0, .time = 1.0 }, .split_projectile = 1 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 12.0,
        .damage = 16.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.0 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 36.0,
        .damage = 48.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.0 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 108.0,
        .damage = 144.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.0 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 324.0,
        .damage = 432.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.0 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 972.0,
        .damage = 1296.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.0 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 2916.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.13 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 8748.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.28 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 26244.0,
        .damage = 34992.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.43 }
    },
    {
        .name = "Dead Leaf",
        .description = "Increases petal health but makes them reload slower.",
        .health = 78732.0,
        .damage = 104976.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_reload_factor = 1.05, .petal_hp_factor = 1.58 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 10.0,
        .damage = 10.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .luck = 0.1 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 30.0,
        .damage = 30.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .luck = 0.2 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 90.0,
        .damage = 90.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .luck = 0.3 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 270.0,
        .damage = 270.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .luck = 0.4 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 810.0,
        .damage = 810.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .luck = 0.5 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 2430.0,
        .damage = 2430.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .luck = 0.6 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 7290.0,
        .damage = 7290.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .luck = 0.7 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 21870.0,
        .damage = 21870.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .luck = 0.8 }
    },
    {
        .name = "Clover",
        .description = "Increases your luck.",
        .health = 65610.0,
        .damage = 65610.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .luck = 0.9 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .totem_heal = 1.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .totem_heal = 3.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .totem_heal = 9.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .totem_heal = 27.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .totem_heal = 81.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .totem_heal = 243.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .totem_heal = 729.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .totem_heal = 2187.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Totem",
        .description = "Grants health regen for a duration after killing a mob.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .totem_heal = 2187.0, .totem_seconds = 10.0 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .root_armor = 2.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .root_armor = 7.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .root_armor = 22.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .root_armor = 67.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .root_armor = 202.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .root_armor = 607.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .root_armor = 1822.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .root_armor = 5467.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Root",
        .description = "Provides layers of additional armor over time.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .root_armor = 5467.5, .root_interval = 2.0, .root_max_stacks = 10 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 20.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .lightning_damage = 12.0, .lightning_chains = 2 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 60.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .lightning_damage = 36.0, .lightning_chains = 3 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 180.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .lightning_damage = 108.0, .lightning_chains = 4 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 540.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .lightning_damage = 324.0, .lightning_chains = 5 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 1620.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .lightning_damage = 972.0, .lightning_chains = 6 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 4860.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .lightning_damage = 2916.0, .lightning_chains = 7 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 14580.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .lightning_damage = 8748.0, .lightning_chains = 8 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 43740.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .lightning_damage = 26244.0, .lightning_chains = 9 }
    },
    {
        .name = "Lightning",
        .description = "Strikes several nearby enemies.",
        .health = 131220.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .lightning_damage = 78732.0, .lightning_chains = 10 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .lightning_damage = 12.0, .lightning_chains = 2, .battery_charges = 3 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .lightning_damage = 36.0, .lightning_chains = 3, .battery_charges = 3 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .lightning_damage = 108.0, .lightning_chains = 4, .battery_charges = 3 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .lightning_damage = 324.0, .lightning_chains = 5, .battery_charges = 3 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .lightning_damage = 972.0, .lightning_chains = 6, .battery_charges = 3 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .lightning_damage = 2916.0, .lightning_chains = 7, .battery_charges = 3 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .lightning_damage = 8748.0, .lightning_chains = 8, .battery_charges = 3 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .lightning_damage = 26244.0, .lightning_chains = 9, .battery_charges = 3 }
    },
    {
        .name = "Battery",
        .description = "Discharges electricity whenever the flower gets hit.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .lightning_damage = 78732.0, .lightning_chains = 10, .battery_charges = 3 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 10.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .lightning_damage = 50.0, .lightning_chains = 1 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 30.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .lightning_damage = 150.0, .lightning_chains = 1 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 90.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .lightning_damage = 450.0, .lightning_chains = 1 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 270.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .lightning_damage = 1350.0, .lightning_chains = 1 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 810.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .lightning_damage = 4050.0, .lightning_chains = 1 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 2430.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .lightning_damage = 12150.0, .lightning_chains = 1 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 7290.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .lightning_damage = 36450.0, .lightning_chains = 1 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 21870.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .lightning_damage = 109350.0, .lightning_chains = 1 }
    },
    {
        .name = "Laser",
        .description = "Pew pew.",
        .health = 65610.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .lightning_damage = 328050.0, .lightning_chains = 1 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 5.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 50.0, .aura_duration = 10.0, .lightning_damage = 5.0 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 15.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 60.0, .aura_duration = 10.0, .lightning_damage = 15.0 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 45.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 70.0, .aura_duration = 10.0, .lightning_damage = 45.0 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 135.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 80.0, .aura_duration = 10.0, .lightning_damage = 135.0 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 405.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 100.0, .aura_duration = 10.0, .lightning_damage = 405.0 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 1215.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 150.0, .aura_duration = 10.0, .lightning_damage = 1215.0 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 3645.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 200.0, .aura_duration = 10.0, .lightning_damage = 3645.0 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 10935.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 250.0, .aura_duration = 10.0, .lightning_damage = 10935.0 }
    },
    {
        .name = "Electric Web",
        .description = "A little less sticky, but a bit more shocking.",
        .health = 32805.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.5, .aura_radius = 300.0, .aura_duration = 10.0, .lightning_damage = 32805.0 }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 75.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 50.0,.time=1.0} }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 225.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 150.0,.time=1.0} }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 675.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 450.0,.time=1.0} }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 2025.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 1350.0,.time=1.0} }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 6075.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 4050.0,.time=1.0} }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 18225.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 12150.0,.time=1.0} }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 54675.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 36450.0,.time=1.0} }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 164025.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 109350.0,.time=1.0} }
    },
    {
        .name = "Uranium",
        .description = "Periodically releases radiation.",
        .health = 492075.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .aura_radius = 80.0, .aura_duration = 1.5, .poison_damage = {.damage = 328050.0,.time=1.0} }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .damage_delay_seconds = 3.0 }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .damage_delay_seconds = 6.0 }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .damage_delay_seconds = 9.0 }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_delay_seconds = 12.0 }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_delay_seconds = 15.0 }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_delay_seconds = 18.0 }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_delay_seconds = 21.0 }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_delay_seconds = 24.0 }
    },
    {
        .name = "Sponge",
        .description = "Spreads damage taken over time.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_delay_seconds = 27.0 }
    },
    // Fang: wiki HP=1 glass; Ashflor uses Basic ×3 durability so it can
    // stay in contact and actually deliver on-hit heal (尖牙正常耐久).
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 10.0,
        .damage = 15.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .on_hit_heal_ratio = 0.35 }
    },
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 30.0,
        .damage = 45.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .on_hit_heal_ratio = 0.35 }
    },
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 90.0,
        .damage = 135.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .on_hit_heal_ratio = 0.35 }
    },
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 270.0,
        .damage = 405.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .on_hit_heal_ratio = 0.35 }
    },
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 810.0,
        .damage = 1215.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .on_hit_heal_ratio = 0.35 }
    },
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 2430.0,
        .damage = 3645.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .on_hit_heal_ratio = 0.35 }
    },
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 7290.0,
        .damage = 10935.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .on_hit_heal_ratio = 0.202 }
    },
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 21870.0,
        .damage = 32805.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .on_hit_heal_ratio = 0.117 }
    },
    {
        .name = "Fang",
        .description = "Heals based on damage dealt by this petal.",
        .health = 65610.0,
        .damage = 98415.0,
        .radius = 8.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .on_hit_heal_ratio = 0.067 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 25.0,
        .damage = 5.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 7.0 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 75.0,
        .damage = 15.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 21.0 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 225.0,
        .damage = 45.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 63.0 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 675.0,
        .damage = 135.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 189.0 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 2025.0,
        .damage = 405.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 567.0 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 6075.0,
        .damage = 1215.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 1701.0 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 18225.0,
        .damage = 3645.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 2946.22 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 54675.0,
        .damage = 10935.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 5103.0 }
    },
    {
        .name = "Shell",
        .description = "Adds a temporary shield to yourself or allies.",
        .health = 164025.0,
        .damage = 32805.0,
        .radius = 11.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 1.5, .burst_heal = 8838.66 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 10.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 30.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 90.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 270.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 810.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 2430.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 7290.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 21870.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Claw",
        .description = "Deals extra damage if the victim is above 80% health.",
        .health = 65610.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .execute_threshold = 0.80, .execute_factor = 21.0 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 1.0,
        .damage = 22.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 3.0,
        .damage = 66.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 9.0,
        .damage = 198.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 27.0,
        .damage = 594.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 81.0,
        .damage = 1782.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 243.0,
        .damage = 5346.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 729.0,
        .damage = 16038.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 2187.0,
        .damage = 48114.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Glass",
        .description = "Phases through enemies. Limited hit interval.",
        .health = 6561.0,
        .damage = 144342.0,
        .radius = 9.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .hit_interval_seconds = 1.0, .phase_through = 1 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 10.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .taunt_radius = 300.0 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 30.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .taunt_radius = 600.0 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 90.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .taunt_radius = 900.0 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 270.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .taunt_radius = 1200.0 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 810.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .taunt_radius = 1500.0 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 2430.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .taunt_radius = 1800.0 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 7290.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .taunt_radius = 2100.0 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 21870.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .taunt_radius = 2400.0 }
    },
    {
        .name = "Light Bulb",
        .description = "Draws aggro from mobs.",
        .health = 65610.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .taunt_radius = 2700.0 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 5.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .aggro_shrink = 0.25 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 15.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .aggro_shrink = 0.438 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 45.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .aggro_shrink = 0.578 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 135.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .aggro_shrink = 0.684 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 405.0,
        .damage = 405.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .aggro_shrink = 0.763 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 1215.0,
        .damage = 1215.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .aggro_shrink = 0.822 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 3645.0,
        .damage = 3645.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .aggro_shrink = 0.867 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 10935.0,
        .damage = 10935.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .aggro_shrink = 0.9 }
    },
    {
        .name = "Poo",
        .description = "Makes mobs less likely to attack you.",
        .health = 32805.0,
        .damage = 32805.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .aggro_shrink = 0.925 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 10.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 30.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 90.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 270.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 810.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 2430.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 7290.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 21870.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Ankh",
        .description = "When destroyed, teleports the flower back.",
        .health = 65610.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 256.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.5, .ankh_mark = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Shovel",
        .description = "Become untargetable but unable to use petals briefly.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .shovel_burrow = 1, .defend_only = 1 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 1.5 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 2.1 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 2.9 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 4.1 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 5.8 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 8.1 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 11.3 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 15.8 }
    },
    {
        .name = "Bandage",
        .description = "Delays death for a limited time.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .equipment = EquipmentFlags::kBandage, .death_delay_seconds = 22.1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 1000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .aura_radius = 30.0, .wax_block = 1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 3000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .aura_radius = 60.0, .wax_block = 1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 9000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .aura_radius = 90.0, .wax_block = 1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 27000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .aura_radius = 120.0, .wax_block = 1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 81000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .aura_radius = 150.0, .wax_block = 1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 243000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .aura_radius = 180.0, .wax_block = 1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 729000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .aura_radius = 210.0, .wax_block = 1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 2187000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .aura_radius = 240.0, .wax_block = 1 }
    },
    {
        .name = "Wax",
        .description = "Don't ask where it came from.",
        .health = 6561000.0,
        .damage = 0.0,
        .radius = 14.0,
        .reload = 30.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .aura_radius = 270.0, .wax_block = 1 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 10.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .overheal_to_shield = 0.05 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 30.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .overheal_to_shield = 0.1 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 90.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .overheal_to_shield = 0.15 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 270.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .overheal_to_shield = 0.2 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 810.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .overheal_to_shield = 0.25 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 2430.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .overheal_to_shield = 0.3 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 7290.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .overheal_to_shield = 0.35 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 21870.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .overheal_to_shield = 0.4 }
    },
    {
        .name = "Amulet",
        .description = "Converts a percentage of overheal into shields.",
        .health = 65610.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .overheal_to_shield = 0.45 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .privet_extra_poison = 0.2 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .privet_extra_poison = 0.6 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .privet_extra_poison = 1.8 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .privet_extra_poison = 5.4 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .privet_extra_poison = 16.2 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .privet_extra_poison = 48.6 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .privet_extra_poison = 145.8 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .privet_extra_poison = 437.4 }
    },
    {
        .name = "Privet Berry",
        .description = "Deals extra poison based on existing poison DPS.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .privet_extra_poison = 1312.2 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 5.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kCommon,
        .attributes = { .lightning_damage = 30.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 15.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kUnusual,
        .attributes = { .lightning_damage = 90.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 45.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kRare,
        .attributes = { .lightning_damage = 270.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 135.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kEpic,
        .attributes = { .lightning_damage = 810.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 405.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kLegendary,
        .attributes = { .lightning_damage = 2430.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 1215.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kMythic,
        .attributes = { .lightning_damage = 7290.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 3645.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kUltra,
        .attributes = { .lightning_damage = 21870.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 10935.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kSuper,
        .attributes = { .lightning_damage = 65610.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Blueberries",
        .description = "It goes poof. Extra ingredient: lightning.",
        .health = 32805.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 1.2,
        .count = 4,
        .rarity = RarityID::kUnique,
        .attributes = { .lightning_damage = 196830.0, .lightning_chains = 1, .split_projectile = 1 }
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 1.0,
        .damage = 1.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 3.0,
        .damage = 3.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 9.0,
        .damage = 9.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 27.0,
        .damage = 27.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 81.0,
        .damage = 81.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 243.0,
        .damage = 243.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 729.0,
        .damage = 729.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 2187.0,
        .damage = 2187.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Fragment",
        .description = "Perhaps this could be used to forge something great?",
        .health = 6561.0,
        .damage = 6561.0,
        .radius = 8.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 13.0,
        .damage = 19.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 39.0,
        .damage = 57.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 117.0,
        .damage = 171.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 351.0,
        .damage = 513.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 1053.0,
        .damage = 1539.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 3159.0,
        .damage = 4617.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 9477.0,
        .damage = 13851.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 28431.0,
        .damage = 41553.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Cogwheel",
        .description = "Doesn't seem to do much...",
        .health = 85293.0,
        .damage = 124659.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 25.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mana_gen = 8.2, .mana_cap = 57.8 }
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 75.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mana_gen = 16.4, .mana_cap = 115.6 }
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 225.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mana_gen = 32.8, .mana_cap = 231.2 }
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 675.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mana_gen = 65.6, .mana_cap = 462.4 }
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 2025.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mana_gen = 131.2, .mana_cap = 924.8 }
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 6075.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mana_gen = 262.4, .mana_cap = 1849.6 }
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 18225.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mana_gen = 524.8, .mana_cap = 3699.2 }
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 54675.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mana_gen = 1049.6, .mana_cap = 7398.4 }
    },
    {
        .name = "Mana Orb",
        .description = "Creates mana.",
        .health = 164025.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mana_gen = 2099.2, .mana_cap = 14796.8 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 12.0,
        .damage = 16.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 0, .mana_gen = 2.5, .mana_cap = 57.8 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 36.0,
        .damage = 48.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 0, .mana_gen = 5.0, .mana_cap = 115.6 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 108.0,
        .damage = 144.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 0, .mana_gen = 10.0, .mana_cap = 231.2 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 324.0,
        .damage = 432.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 0, .mana_gen = 20.0, .mana_cap = 462.4 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 972.0,
        .damage = 1296.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 0, .mana_gen = 40.0, .mana_cap = 924.8 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 2916.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 0, .mana_gen = 80.0, .mana_cap = 1849.6 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 8748.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 0, .mana_gen = 160.0, .mana_cap = 3699.2 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 26244.0,
        .damage = 34992.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 0, .mana_gen = 320.0, .mana_cap = 7398.4 }
    },
    {
        .name = "Magic Leaf",
        .description = "Gathers mana from the environment passively.",
        .health = 78732.0,
        .damage = 104976.0,
        .radius = 10.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 0, .mana_gen = 640.0, .mana_cap = 14796.8 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 15.0,
        .damage = 7.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_health = 0.0, .mana_cap = 115.5 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 45.0,
        .damage = 21.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 0.0, .mana_cap = 231.0 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 135.0,
        .damage = 63.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 0.0, .mana_cap = 462.0 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 405.0,
        .damage = 189.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 0.0, .mana_cap = 924.0 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 1215.0,
        .damage = 567.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 0.0, .mana_cap = 1848.0 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 3645.0,
        .damage = 1701.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 0.0, .mana_cap = 3696.0 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 10935.0,
        .damage = 5103.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 0.0, .mana_cap = 7392.0 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 32805.0,
        .damage = 15309.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 0.0, .mana_cap = 14784.0 }
    },
    {
        .name = "Magic Cactus",
        .description = "Increases maximum mana capacity.",
        .health = 98415.0,
        .damage = 45927.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 0.0, .mana_cap = 29568.0 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mana_absorb_per = 0.5 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mana_absorb_per = 0.8 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mana_absorb_per = 1.1 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mana_absorb_per = 1.7 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mana_absorb_per = 2.5 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mana_absorb_per = 3.8 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mana_absorb_per = 5.7 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mana_absorb_per = 8.5 }
    },
    {
        .name = "Magic Cotton",
        .description = "Absorbs damage using mana.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mana_absorb_per = 12.8 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 3.3 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 4.9 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 7.4 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 11.1 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 16.7 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 25.1 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 37.6 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 56.4 }
    },
    {
        .name = "Magic Eye",
        .description = "Consumes half current mana to deal lightning damage.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .lightning_chains = 7, .mana_lightning_per = 84.6 }
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 1.0,
        .damage = 100.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {}
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 3.0,
        .damage = 300.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 9.0,
        .damage = 900.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 27.0,
        .damage = 2700.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 81.0,
        .damage = 8100.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 243.0,
        .damage = 24300.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 729.0,
        .damage = 72900.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 2187.0,
        .damage = 218700.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Magic Stinger",
        .description = "It really hurts, but it's very fragile.",
        .health = 6561.0,
        .damage = 656100.0,
        .radius = 7.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 2.0,
        .damage = 35.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 6.0,
        .damage = 105.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 18.0,
        .damage = 315.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 54.0,
        .damage = 945.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 162.0,
        .damage = 2835.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 486.0,
        .damage = 8505.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 1458.0,
        .damage = 25515.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 4374.0,
        .damage = 76545.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Missile",
        .description = "You can shoot this one... with magic.",
        .health = 13122.0,
        .damage = 229635.0,
        .radius = 8.0,
        .reload = 5.4,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .split_projectile = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mana_spawn_cost = 20.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mana_spawn_cost = 40.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mana_spawn_cost = 80.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mana_spawn_cost = 160.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mana_spawn_cost = 320.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mana_spawn_cost = 640.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mana_spawn_cost = 1280.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mana_spawn_cost = 2560.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Stick",
        .description = "Summons the forces of the wind... with magic.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mana_spawn_cost = 5120.0, .spawns = MobID::kSandstorm, .spawn_count = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Magic Bubble",
        .description = "Physics are for the magicless.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .defend_only = 1 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .flower_evasion = 0.03 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 30.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .flower_evasion = 0.06 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 90.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .flower_evasion = 0.09 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 270.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .flower_evasion = 0.12 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 810.0,
        .damage = 810.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .flower_evasion = 0.15 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 2430.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .flower_evasion = 0.18 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 7290.0,
        .damage = 7290.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .flower_evasion = 0.21 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 21870.0,
        .damage = 21870.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .flower_evasion = 0.24 }
    },
    {
        .name = "Talisman of Evasion",
        .description = "Allows the wearer to anticipate enemy attacks.",
        .health = 65610.0,
        .damage = 65610.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .flower_evasion = 0.27 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Nazar Amulet",
        .description = "Reduces damage by 98% and consumes a charge.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 60.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .nazar_charges = 3, .nazar_mitigation = 0.98 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 40.0,
        .damage = 1.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 9.0, .compass = 1 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 120.0,
        .damage = 3.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 8.0, .compass = 1 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 360.0,
        .damage = 9.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 7.0, .compass = 1 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 1080.0,
        .damage = 27.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 6.0, .compass = 1 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 3240.0,
        .damage = 81.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 5.0, .compass = 1 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 9720.0,
        .damage = 243.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 4.0, .compass = 1 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 29160.0,
        .damage = 729.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 3.0, .compass = 1 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 87480.0,
        .damage = 2187.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 2.0, .compass = 1 }
    },
    {
        .name = "Compass",
        .description = "This might find something interesting...",
        .health = 262440.0,
        .damage = 6561.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 1.0, .compass = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mimic",
        .description = "Copies the petal to the left at the same rarity.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mimic_left = 1 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 10.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .move_speed_factor = 1.25, .powder_max_rarity = 0 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 30.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .move_speed_factor = 1.5, .powder_max_rarity = 1 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 90.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .move_speed_factor = 1.75, .powder_max_rarity = 2 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 270.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .move_speed_factor = 2.0, .powder_max_rarity = 3 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 810.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .move_speed_factor = 2.25, .powder_max_rarity = 4 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 2430.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .move_speed_factor = 2.5, .powder_max_rarity = 5 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 7290.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .move_speed_factor = 2.75, .powder_max_rarity = 6 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 21870.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .move_speed_factor = 3.0, .powder_max_rarity = 6 }
    },
    {
        .name = "Mysterious Powder",
        .description = "Increases movement speed.",
        .health = 65610.0,
        .damage = 0.0,
        .radius = 8.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .move_speed_factor = 3.25, .powder_max_rarity = 6 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_health = 0.1 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 0.2 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 0.3 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 0.4 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 0.5 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 0.6 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 0.7 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 0.8 }
    },
    {
        .name = "Mysterious Relic",
        .description = "Significantly increases maximum health.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 0.9 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 10.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 960.0, .dark_mark = 1 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 30.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 480.0, .dark_mark = 1 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 90.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 240.0, .dark_mark = 1 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 270.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 120.0, .dark_mark = 1 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 810.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 60.0, .dark_mark = 1 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 2430.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 30.0, .dark_mark = 1 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 7290.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 15.0, .dark_mark = 1 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 21870.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 7.5, .dark_mark = 1 }
    },
    {
        .name = "Dark Mark",
        .description = "A dark mark that binds itself to a fallen soul.",
        .health = 65610.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 3.75, .dark_mark = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Corruption",
        .description = "Corrupts one's soul.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .corruption = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Blood Sacrifice",
        .description = "On death, summons a mob to fight again.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .blood_sacrifice = 1 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Pharaoh's Crown",
        .description = "Spawns Undead Beetles to fight at your side.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 1.0, .spawns = (uint8_t)MobID::kBeetle, .spawn_count = 6 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Champion's Crown",
        .description = "Gives flowers around you a Super Basic.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .champion_crown = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Soul Splitter",
        .description = "Fractures your soul into two.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .soul_splitter = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 1.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 3.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 9.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 27.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 81.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 243.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 729.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 2187.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    {
        .name = "Paranormal Goggles",
        .description = "Allows finding some paranormal activity.",
        .health = 6561.0,
        .damage = 0.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .vision_factor = 0.80, .paranormal = 1 }
    },
    // ---- Salve / 药膏：回复其他花瓣耐久 ----
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 15.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .petal_repair = 4.0 }
    },
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 45.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .petal_repair = 12.0 }
    },
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 135.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .petal_repair = 36.0 }
    },
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 405.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .petal_repair = 108.0 }
    },
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 1215.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .petal_repair = 324.0 }
    },
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 3645.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .petal_repair = 972.0 }
    },
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 10935.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .petal_repair = 1680.0 }
    },
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 32805.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .petal_repair = 2916.0 }
    },
    {
        .name = "Salve",
        .description = "Restores durability of your other petals.",
        .health = 98415.0,
        .damage = 0.0,
        .radius = 11.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .petal_repair = 5040.0 }
    }
}};

std::array<struct MobData, MobID::kNumMobs> const MOB_DATA = {{

    {
        .name = "Baby Ant",
        .description = "Weak and defenseless, but big dreams.",
        .rarity = RarityID::kCommon,
        .health = {25.0},
        .damage = 10.0,
        .radius = {14.0},
        .xp = 1,
        .drops = {
            PetalID::kCotton, PetalID::kPoo, PetalID::kMagicCotton,
            PetalID::kLight, PetalID::kLeaf, PetalID::kTwin, PetalID::kRice, PetalID::kTriplet}, 
        .attributes = {}
    },
    {
        .name = "Worker Ant",
        .description = "It's temperamental, probably from working all the time.",
        .rarity = RarityID::kCommon,
        .health = {60.0},
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
        .rarity = RarityID::kCommon,
        .health = {100.0},
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
            PetalID::kOrange,
            PetalID::kBandage,
            PetalID::kWax,
            PetalID::kBlueberries,
            PetalID::kMagicStinger,
            PetalID::kLight,
            PetalID::kStinger,
            PetalID::kTwin
        },
        .attributes = {}
    },
    {
        .name = "Ladybug",
        .description = "Cute and harmless.",
        .rarity = RarityID::kCommon,
        .health = {85.0},
        .damage = 10.0,
        .radius = {30.0},
        .xp = 3,
        .drops = {
            PetalID::kBean,
            PetalID::kTomato,
            PetalID::kBasil,
            PetalID::kClover,
            PetalID::kAmulet,
            PetalID::kLight,
            PetalID::kRose,
            PetalID::kTwin
        },
        .attributes = {}
    },
    {
        .name = "Beetle",
        .description = "It's hungry and flowers are its favorite meal.",
        .rarity = RarityID::kCommon,
        .health = {100.0},
        .damage = 30.0,
        .radius = {35.0},
        .xp = 10,
        .drops = {
            PetalID::kLentil,
            PetalID::kIris, PetalID::kSalt, PetalID::kWing, PetalID::kTriplet,
            PetalID::kMatTwig},
        .attributes = {}
    },
    {
        .name = "Massive Ladybug",
        .description = "Much larger, but still cute.",
        .rarity = RarityID::kEpic,
        .health = {1000.0},
        .damage = 10.0,
        // Was 90 — rarity scale + 1.25 global mult blew up high-tier spawns.
        .radius = {58.0},
        .xp = 400,
        .drops = {
            PetalID::kTotem,
            PetalID::kRose, PetalID::kDahlia, PetalID::kBubble, PetalID::kAzalea, PetalID::kObserver}, 
        .attributes = {}
    },
    {
        .name = "Massive Beetle",
        .description = "Someone overfed this one, you might be next.",
        .rarity = RarityID::kRare,
        .health = {600.0},
        .damage = 35.0,
        // Was 75 — with MOB_RADIUS_MULTIPLIER×rarity scale Legendary filled the FOV.
        .radius = {52.0},
        .xp = 50,
        .drops = {
            PetalID::kLentil,
            PetalID::kIris, PetalID::kWing, PetalID::kBlueIris, PetalID::kTriplet, PetalID::kBeetleEgg, PetalID::kThirdEye}, 
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
            PetalID::kSalve,
            PetalID::kDahlia, PetalID::kWing, PetalID::kYinYang, PetalID::kAzalea
        },
        .attributes = {}
    },
    {
        .name = "Hornet",
        .description = "These aren't quite as nice as the little bees.",
        .rarity = RarityID::kCommon,
        .health = {60.0},
        .damage = 50.0,
        .radius = {40.0},
        .xp = 12,
        .drops = {
            PetalID::kCarrot,
            PetalID::kLightning,
            PetalID::kWax,
            PetalID::kMagicMissile,
            PetalID::kDandelion,
            PetalID::kMissile,
            PetalID::kWing,
            PetalID::kBubble
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
            PetalID::kBroccoli, PetalID::kRoot, PetalID::kMagicCactus,
            PetalID::kStinger, PetalID::kYucca, PetalID::kCactus,
            PetalID::kMatCactusSap, PetalID::kMatVenomBarb},
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
            PetalID::kClay, PetalID::kSoil, PetalID::kGlass,
            PetalID::kHeavy, PetalID::kLight, PetalID::kRock, PetalID::kMeteor,
            PetalID::kMoon},
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
            PetalID::kClay,
            PetalID::kHeavy, PetalID::kRock, PetalID::kHeaviest, PetalID::kMoon, PetalID::kMeteor}, 
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
            PetalID::kGrapes,
            PetalID::kLight, PetalID::kTwin, PetalID::kLeaf, PetalID::kPeas,
            PetalID::kMatLeaf, PetalID::kMatTwig},
        .attributes = {
            .segments = 10
        }
    },
    {
        .name = "Evil Centipede",
        .description = "This one loves flowers.",
        .rarity = RarityID::kCommon,
        .health = {25.0},
        .damage = 10.0,
        .radius = {35.0},
        .xp = 3,
        .drops = {
            PetalID::kGrapes, PetalID::kCorruption,
            PetalID::kIris, PetalID::kPoisonPeas, PetalID::kBlueIris},
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
        // florr: Salt / Powder — Stick belongs on Sandstorm (summons sandstorms).
        .drops = {
            PetalID::kSand, PetalID::kFaster, PetalID::kSalt, PetalID::kPowder
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
            PetalID::kDust, PetalID::kCompass, PetalID::kPowder, PetalID::kStick,
            PetalID::kMagicStick, PetalID::kSand, PetalID::kFaster,
            PetalID::kMatSand, PetalID::kMatSandstormEye},
        .attributes = {}
    },
    {
        .name = "Scorpion",
        .description = "This one stings, now with poison.",
        .rarity = RarityID::kCommon,
        .health = {100.0},
        .damage = 15.0,
        .radius = {35.0},
        .xp = 10,
        .drops = {
            PetalID::kBattery,
            PetalID::kUranium,
            PetalID::kFang,
            PetalID::kPrivetBerry,
            PetalID::kNazar,
            PetalID::kIris,
            PetalID::kPincer,
            PetalID::kTriplet
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
        .rarity = RarityID::kCommon,
        .health = {60.0},
        .damage = 15.0,
        .radius = {15.0},
        .xp = 8,
        .drops = {
            PetalID::kBur, PetalID::kElectricWeb, PetalID::kFang, PetalID::kTalisman,
            PetalID::kStinger, PetalID::kWeb, PetalID::kFaster, PetalID::kTriweb},
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
        .description = "Sleeping sovereign of Ant Hell. Drops Hive Heart and Queen Pheromone.",
        .rarity = RarityID::kCommon,
        .health = {250.0},
        .damage = 10.0,
        .radius = {25.0},
        .xp = 15,
        // Boss table: signature petals only (mats come from trash ants / shop).
        .drops = {
            PetalID::kBloodSacrifice,
            PetalID::kColonyHeart, PetalID::kQueensEmblem, PetalID::kAntEgg, PetalID::kTriplet,
            PetalID::kThirdEye, PetalID::kMatHiveHeart, PetalID::kMatQueenPheromone},
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
            PetalID::kDahlia, PetalID::kWing, PetalID::kBubble, PetalID::kYggdrasil,
            PetalID::kMatPollenDust
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
            PetalID::kDice, PetalID::kDomino, PetalID::kCard, PetalID::kPokerChip,
            PetalID::kSquare},
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
        .drops = {
            PetalID::kSawblade, PetalID::kFragment, PetalID::kCogwheel,
            PetalID::kIronGuard, PetalID::kMatSteel, PetalID::kMatGear},
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
        .drops = {
            PetalID::kStoneHeart, PetalID::kIronGuard, PetalID::kMatSteel, PetalID::kMoon
        },
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
        .drops = {
            PetalID::kSawblade, PetalID::kFragment,
            PetalID::kRustNeedle, PetalID::kThornTrap, PetalID::kMatGear},
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
        .drops = {
            PetalID::kSawblade, PetalID::kMimic, PetalID::kRelic,
            PetalID::kAncientCoin, PetalID::kGuardEgg, PetalID::kMatGear},
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
        .drops = { PetalID::kAncientCoin, PetalID::kKingsOath, PetalID::kMatCapitalBadge },
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
        .drops = { PetalID::kRoyalRose, PetalID::kRoyalBanner, PetalID::kMatHolyShard },
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
        .drops = {
            PetalID::kIronGuard, PetalID::kStoneHeart, PetalID::kGuardEgg, PetalID::kMatCapitalBadge
        },
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
        .drops = {
            PetalID::kMagicEye, PetalID::kSoulSplitter,
            PetalID::kPhantomEye, PetalID::kKingsOath, PetalID::kMatHolyShard},
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
        .drops = {
            PetalID::kRoyalSpear, PetalID::kPhantomEye, PetalID::kKingsOath, PetalID::kMatRoyalSeal
        },
        .attributes = { .aggro_radius = 720 }
    },
    {
        // Slice C: full phase boss. Present for ID/art; not in zone spawn pools.
        .name = "Fallen King",
        .description = "Sleeping sovereign of the Capital. Drops Sovereign Core and Royal Seal.",
        .rarity = RarityID::kUnique,
        .health = {2500.0},
        .damage = 28.0,
        .radius = {112.2},
        .xp = 200,
        .drops = {
            PetalID::kCrownFragment, PetalID::kKingsOath, PetalID::kRoyalSpear, PetalID::kMatSovereignCore,
            PetalID::kMatRoyalSeal
        },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Jungle mobs ----
    {
        .name = "Leaf Beetle",
        .description = "A beetle wearing leaf armor.",
        .rarity = RarityID::kCommon,
        .health = {35.0}, .damage = 10.0, .radius = {41.5}, .xp = 4,
        .drops = {
            PetalID::kSalve,
            PetalID::kDeadLeaf, PetalID::kMagicLeaf,
            PetalID::kLeafBlade, PetalID::kEmeraldShell, PetalID::kMonstera, PetalID::kMatResin},
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
        .drops = {
            PetalID::kVineWhip, PetalID::kThornRing, PetalID::kMonstera, PetalID::kMatVine
        },
        .attributes = { .aggro_radius = 600 }
    },
    {
        .name = "Jungle Spider",
        .description = "A canopy spider with a venomous bite.",
        .rarity = RarityID::kUnusual,
        .health = {45.0}, .damage = 14.0, .radius = {31.7}, .xp = 7,
        .drops = { PetalID::kSpiderFang, PetalID::kPoisonSpore, PetalID::kMatVenomSac },
        .attributes = { .aggro_radius = 650 }
    },
    {
        .name = "Emerald Mantis",
        .description = "An emerald predator with scythe limbs.",
        .rarity = RarityID::kCommon,
        .health = {100.0}, .damage = 20.0, .radius = {45.1}, .xp = 16,
        .drops = { PetalID::kMantisClaw, PetalID::kLeafBlade },
        .attributes = { .aggro_radius = 700 }
    },
    {
        .name = "Poison Toad",
        .description = "A bulky toad dripping yellow toxin.",
        .rarity = RarityID::kRare,
        .health = {110.0}, .damage = 12.0, .radius = {41.5}, .xp = 14,
        .drops = { PetalID::kPoisonSpore, PetalID::kToxicHeart, PetalID::kMatVenomSac },
        .attributes = { .aggro_radius = 580, .poison_damage = { .damage = 5.0, .time = 3.0 } }
    },
    {
        .name = "Spore Mushroom",
        .description = "A slow mushroom that sheds spores as it walks.",
        .rarity = RarityID::kUnusual,
        .health = {55.0}, .damage = 9.0, .radius = {30.5}, .xp = 6,
        .drops = {
            PetalID::kPoisonSpore, PetalID::kSporeCloud, PetalID::kMatSpore, PetalID::kMatCorruptSpore
        },
        .attributes = { .aggro_radius = 500 }
    },
    {
        .name = "Carnivorous Plant",
        .description = "Looks like foliage until it opens its jaws.",
        .rarity = RarityID::kRare,
        .health = {120.0}, .damage = 18.0, .radius = {46.4}, .xp = 18,
        .drops = {
            PetalID::kVineWhip, PetalID::kToxicHeart, PetalID::kMonstera, PetalID::kMatCorruptSpore
        },
        .attributes = { .aggro_radius = 620 }
    },
    {
        .name = "River Crab",
        .description = "A heavy blue-green crab of the jungle riverbanks.",
        .rarity = RarityID::kUnusual,
        .health = {85.0}, .damage = 15.0, .radius = {37.8}, .xp = 10,
        .drops = {
            PetalID::kClaw,
            PetalID::kEmeraldShell, PetalID::kThornRing},
        .attributes = { .aggro_radius = 560 }
    },
    {
        .name = "Stone Guardian",
        .description = "A vine-wrapped stone sentinel of the ruins.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 16.0, .radius = {56.1}, .xp = 28,
        .drops = { PetalID::kAncientSeed, PetalID::kEmeraldShell, PetalID::kMoon },
        .attributes = { .aggro_radius = 680 }
    },
    {
        .name = "Jungle Wasp",
        .description = "A striped wasp that never leaves the canopy alone.",
        .rarity = RarityID::kCommon,
        .health = {60.0}, .damage = 50.0, .radius = {29.3}, .xp = 8,
        .drops = { PetalID::kJungleNeedle, PetalID::kSpiderFang },
        .attributes = { .aggro_radius = 720 }
    },
    {
        .name = "Ancient Treant",
        .description = "Sleeping sovereign of the Jungle. Drops Heartwood Chip and Resin.",
        .rarity = RarityID::kUnique,
        .health = {2800.0}, .damage = 26.0, .radius = {115.9}, .xp = 220,
        .drops = {
            PetalID::kAncientSeed, PetalID::kHeartwood, PetalID::kVineWhip, PetalID::kMonstera,
            // Golden Leaf Ultra only (same scarce Ultra weight as Molten Overlord)
            PetalID::kGoldenLeafUltra,
            PetalID::kMatHeartwoodChip, PetalID::kMatResin
        },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Ocean mobs ----
    {
        .name = "Reef Stone",
        .description = "A pale reef boulder resting on the sandy seabed.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {26.8}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy, PetalID::kMoon },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Sea Fry",
        .description = "A timid little fish that only fights back.",
        .rarity = RarityID::kCommon,
        .health = {14.0}, .damage = 10.0, .radius = {31.7}, .xp = 2,
        .drops = { PetalID::kBubble, PetalID::kLeaf, PetalID::kMatWaterDrop },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Moon Jelly",
        .description = "A translucent drifter that drifts toward warm light.",
        .rarity = RarityID::kCommon,
        .health = {75.0}, .damage = 25.0, .radius = {41.5}, .xp = 4,
        .drops = {
            PetalID::kAir, PetalID::kRubber, PetalID::kSponge, PetalID::kMagicBubble,
            PetalID::kBubble, PetalID::kMatWaterDrop, PetalID::kMatSeaSalt},
        .attributes = { .aggro_radius = 220 }
    },
    {
        .name = "Coral Star",
        .description = "A slow orange starfish that guards its coral.",
        .rarity = RarityID::kCommon,
        .health = {50.0}, .damage = 20.0, .radius = {34.2}, .xp = 5,
        .drops = {
            PetalID::kShell,
            PetalID::kRose, PetalID::kMatSeashell, PetalID::kMatCoral},
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Reef Crab",
        .description = "A broad-shelled crab with two heavy claws.",
        .rarity = RarityID::kCommon,
        .health = {75.0}, .damage = 25.0, .radius = {39.0}, .xp = 6,
        .drops = {
            PetalID::kClaw,
            PetalID::kPincer, PetalID::kHeavy, PetalID::kRock, PetalID::kMatSeashell},
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
        .rarity = RarityID::kCommon,
        .health = {100.0}, .damage = 10.0, .radius = {36.6}, .xp = 9,
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
        .drops = {
            PetalID::kCutter, PetalID::kHeaviest, PetalID::kThirdEye, PetalID::kMissile,
            PetalID::kMatAbyssPearl
        },
        .attributes = { .aggro_radius = 450 }
    },
    {
        .name = "Abyss Maw",
        .description = "Sleeping sovereign of the Ocean. Drops Tide Core and Abyss Pearl.",
        .rarity = RarityID::kUnique,
        .health = {2700.0}, .damage = 28.0, .radius = {115.9}, .xp = 220,
        .drops = {
            PetalID::kLotus, PetalID::kYinYang, PetalID::kHeaviest, PetalID::kMoon,
            PetalID::kMatTideCore, PetalID::kMatAbyssPearl
        },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Snow mobs ----
    {
        .name = "Frost Stone",
        .description = "A blue ice boulder under a thin blanket of snow.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy, PetalID::kSalt, PetalID::kMoon },
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
        .drops = {
            PetalID::kFaster, PetalID::kBubble, PetalID::kMoon, PetalID::kMatBlizzardEye
        },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Glacier Beast",
        .description = "A heavy polar beast with two stubby horns.",
        .rarity = RarityID::kUnusual,
        .health = {85.0}, .damage = 20.0, .radius = {48.8}, .xp = 12,
        .drops = {
            PetalID::kHeavy, PetalID::kHeaviest, PetalID::kBone, PetalID::kMatColdIron,
            PetalID::kMatIceScale
        },
        .attributes = { .aggro_radius = 360 }
    },
    {
        .name = "Frost Yeti",
        .description = "A long-armed giant that stalks the frost rifts.",
        .rarity = RarityID::kRare,
        .health = {140.0}, .damage = 20.0, .radius = {54.9}, .xp = 25,
        .drops = {
            PetalID::kBone, PetalID::kHeaviest, PetalID::kYggdrasil, PetalID::kRice,
            PetalID::kMatColdIron
        },
        .attributes = { .aggro_radius = 420 }
    },
    {
        .name = "Ice Golem",
        .description = "A slow colossus of stacked ice and frozen stone.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 22.0, .radius = {61.0}, .xp = 35,
        .drops = {
            PetalID::kRock, PetalID::kHeaviest, PetalID::kThirdEye, PetalID::kLotus,
            PetalID::kMatIceScale, PetalID::kMatColdIron, PetalID::kMoon
        },
        .attributes = { .aggro_radius = 380 }
    },
    {
        .name = "Winter Sovereign",
        .description = "Sleeping sovereign of the Snow. Drops Permafrost Heart and Blizzard Eye.",
        .rarity = RarityID::kUnique,
        .health = {2700.0}, .damage = 28.0, .radius = {115.9}, .xp = 220,
        .drops = {
            PetalID::kLotus, PetalID::kYinYang, PetalID::kHeaviest, PetalID::kYggdrasil,
            PetalID::kMatPermafrostHeart, PetalID::kMatBlizzardEye
        },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Volcano mobs ----
    {
        .name = "Ember Stone",
        .description = "A cooling volcanic rock with dull red cracks.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = {
            PetalID::kRock, PetalID::kHeavy, PetalID::kSalt, PetalID::kMatSulfur,
            PetalID::kMoon
        },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Ash Mite",
        .description = "A grey mite that skitters through the volcanic ash.",
        .rarity = RarityID::kCommon,
        .health = {14.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = {
            PetalID::kLight, PetalID::kPollen, PetalID::kLeaf, PetalID::kMatFireCrystal
        },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Cinder Moth",
        .description = "An ash-grey moth that flutters away from trouble.",
        .rarity = RarityID::kCommon,
        .health = {20.0}, .damage = 10.0, .radius = {23.2}, .xp = 3,
        .drops = {
            PetalID::kLightning,
            PetalID::kLaser,
            PetalID::kLightBulb,
            PetalID::kManaOrb,
            PetalID::kGoggles,
            PetalID::kWing,
            PetalID::kFaster,
            PetalID::kDandelion
        },
        .attributes = { .aggro_radius = 250 }
    },
    {
        .name = "Lava Beetle",
        .description = "A beetle with glowing seams running across its shell.",
        .rarity = RarityID::kCommon,
        .health = {36.0}, .damage = 16.0, .radius = {35.4}, .xp = 5,
        .drops = {
            PetalID::kBeetleEgg, PetalID::kHeavy, PetalID::kRock, PetalID::kMatLavaRock
        },
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
        .drops = {
            PetalID::kRock, PetalID::kHeaviest, PetalID::kBone, PetalID::kCutter,
            PetalID::kMatObsidian, PetalID::kMoon
        },
        .attributes = { .aggro_radius = 380 }
    },
    {
        .name = "Lava Colossus",
        .description = "A hulking lava beast that barrels straight ahead.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 22.0, .radius = {61.0}, .xp = 35,
        .drops = { PetalID::kHeaviest, PetalID::kThirdEye, PetalID::kCutter },
        .attributes = { .aggro_radius = 440 }
    },
    {
        .name = "Molten Overlord",
        .description = "Sleeping sovereign of the Volcano. Drops Leyline Core and Phoenix Ember.",
        .rarity = RarityID::kUnique,
        .health = {2750.0}, .damage = 28.0, .radius = {115.9}, .xp = 220,
        .drops = {
            PetalID::kYinYang, PetalID::kHeaviest, PetalID::kThirdEye,
            // Golden Leaf: Ultra only on this Unique boss (DROP_RARITY_WEIGHT Ultra≈0.02)
            PetalID::kGoldenLeafUltra,
            PetalID::kMatLeylineCore, PetalID::kMatPhoenixEmber
        },
        .attributes = { .aggro_radius = 900 }
    },
    // ---- Desert mobs (new) ----
    // Cactus / Sandstorm / Scorpion / DesertCentipede above are reused unchanged.
    {
        .name = "Dune Stone",
        .description = "A flat wind-worn sandstone with a single eroded groove.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {28.1}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy, PetalID::kMoon },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Sand Mite",
        .description = "A tiny mite that crawls across the hot sand.",
        .rarity = RarityID::kCommon,
        .health = {12.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = {
            PetalID::kLight, PetalID::kSand, PetalID::kAntennae, PetalID::kMatSand
        },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Dune Beetle",
        .description = "A broad-backed desert beetle with a sand-tan shell.",
        .rarity = RarityID::kCommon,
        .health = {38.0}, .damage = 16.0, .radius = {35.4}, .xp = 5,
        .drops = {
            PetalID::kAnkh, PetalID::kShovel,
            PetalID::kBeetleEgg, PetalID::kHeavy, PetalID::kSand, PetalID::kMatSand,
            PetalID::kMatCarapace},
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
        .drops = {
            PetalID::kStinger, PetalID::kWing, PetalID::kTringer, PetalID::kMatVenomBarb
        },
        .attributes = { .aggro_radius = 360 }
    },
    {
        .name = "Dune Titan",
        .description = "A colossal desert carapace that moves with glacial weight.",
        .rarity = RarityID::kEpic,
        .health = {180.0}, .damage = 22.0, .radius = {61.0}, .xp = 35,
        .drops = {
            PetalID::kHeaviest, PetalID::kBeetleEgg, PetalID::kBone, PetalID::kThirdEye,
            PetalID::kMatCarapace
        },
        .attributes = { .aggro_radius = 400 }
    },
    {
        .name = "Sandworm Monarch",
        .description = "Sleeping sovereign of the Desert. Drops Sandworm Core and Sandstorm Eye.",
        .rarity = RarityID::kUnique,
        .health = {2750.0}, .damage = 28.0, .radius = {115.9}, .xp = 220,
        .drops = {
            PetalID::kBone, PetalID::kHeaviest, PetalID::kYinYang, PetalID::kYucca,
            PetalID::kMatSandwormCore, PetalID::kMatSandstormEye
        },
        .attributes = { .aggro_radius = 900, .segments = 10 }
    },
    // ---- Spawn-point fillers (biomes that lacked a stationary + passive pair) ----
    {
        .name = "Jungle Stone",
        .description = "A moss-caked stone half swallowed by the jungle floor.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {28.1}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy, PetalID::kMoon },
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    {
        .name = "Jungle Crawler",
        .description = "A tiny crawler that keeps to the leaf litter.",
        .rarity = RarityID::kCommon,
        .health = {14.0}, .damage = 10.0, .radius = {29.3}, .xp = 2,
        .drops = {
            PetalID::kLeaf, PetalID::kLight, PetalID::kPollen, PetalID::kMatVine
        },
        .attributes = { .aggro_radius = 0 }
    },
    {
        .name = "Royal Boulder",
        .description = "A squared-off block of palace masonry, long since cracked.",
        .rarity = RarityID::kCommon,
        .health = {25.0}, .damage = 10.0, .radius = {28.1}, .xp = 2,
        .drops = { PetalID::kRock, PetalID::kHeavy, PetalID::kMoon },
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
        .description = "Sleeping sovereign of the Garden. Drops Ancient Heart and Royal Jelly.",
        .rarity = RarityID::kUnique,
        .health = {2600.0}, .damage = 26.0, .radius = {112.2}, .xp = 200,
        .drops = {
            PetalID::kBeetleEgg, PetalID::kThirdEye, PetalID::kTriplet, PetalID::kYinYang,
            PetalID::kHeaviest, PetalID::kMatAncientHeart, PetalID::kMatRoyalJelly
        },
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
            PetalID::kPlank,
            PetalID::kTunnelStone, PetalID::kHiveDisc, PetalID::kMatAntShell, PetalID::kBone},
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
        .description = "A friendly treasure chest. Break it open for loot.",
        .rarity = RarityID::kUnusual,
        .health = {3200.0},
        .damage = 0.0,
        .radius = {30.0},
        .xp = 0,
        .drops = {},
        .attributes = { .stationary = 1 }
    },
    {
        .name = "Training Dummy",
        .description = "A stationary practice target. Shows hit damage and DPS, resets after 3 seconds without damage, and gives no rewards.",
        .rarity = RarityID::kCommon,
        .health = {1000.0},
        .damage = 0.0,
        .radius = {35.2},
        .xp = 0,
        .drops = {},
        .attributes = { .aggro_radius = 0, .stationary = 1 }
    },
    // ---- Ant Nest factions (Fire left / Termite right) ----
    {
        .name = "Baby Fire Ant",
        .description = "Weak and defenseless, but on fireeeeeeeeeeeeee.",
        .rarity = RarityID::kCommon,
        .health = {25.0},
        .damage = 10.0,
        .radius = {14.0},
        .xp = 1,
        .drops = {
            PetalID::kLight, PetalID::kYucca, PetalID::kLeaf, PetalID::kMatAntShell
        },
        .attributes = {}
    },
    {
        .name = "Worker Fire Ant",
        .description = "This ant is on fireeeeeeeeeeeeee.",
        .rarity = RarityID::kCommon,
        .health = {62.5},
        .damage = 20.0,
        .radius = {14.0},
        .xp = 1,
        .drops = {
            PetalID::kYucca, PetalID::kCorn, PetalID::kMatAntShell, PetalID::kMatFormicAcid
        },
        .attributes = {}
    },
    {
        .name = "Soldier Fire Ant",
        .description = "It's got wings and it's ready to use them. And it's on fire.",
        .rarity = RarityID::kCommon,
        .health = {100.0},
        .damage = 20.0,
        .radius = {14.0},
        .xp = 1,
        .drops = {
            PetalID::kYucca, PetalID::kBone, PetalID::kMatAntShell, PetalID::kWing
        },
        .attributes = { .aggro_radius = 650 }
    },
    {
        .name = "Baby Termite",
        .description = "Weak and defenseless, but big dreams. These creatures appear to be connected somehow.",
        .rarity = RarityID::kCommon,
        .health = {25.0},
        .damage = 10.0,
        .radius = {14.0},
        .xp = 1,
        .drops = {
            PetalID::kLight, PetalID::kLeaf, PetalID::kBone, PetalID::kTriangle
        },
        .attributes = { .share_damage = 1 }
    },
    {
        .name = "Worker Termite",
        .description = "It's a bit temperamental, probably from working all the time. These creatures appear to be connected somehow.",
        .rarity = RarityID::kCommon,
        .health = {60.0},
        .damage = 10.0,
        .radius = {14.0},
        .xp = 1,
        .drops = {
            PetalID::kBone, PetalID::kTriangle, PetalID::kTriangleUnusual, PetalID::kMatAntShell
        },
        .attributes = { .share_damage = 1 }
    },
    {
        .name = "Soldier Termite",
        .description = "It's got wings and it's ready to use them. These creatures appear to be connected somehow.",
        .rarity = RarityID::kCommon,
        .health = {200.0},
        .damage = 10.0,
        .radius = {14.0},
        .xp = 1,
        .drops = {
            PetalID::kTriangle, PetalID::kTriangleUnusual, PetalID::kTriangleRare, PetalID::kTriangleEpic,
            PetalID::kTriangleLegendary, PetalID::kTriangleMythic, PetalID::kTriangleUltra, PetalID::kBone
        },
        .attributes = { .aggro_radius = 420, .share_damage = 1 }
    }
}};

uint8_t is_chase_loot_petal(PetalID::T id) {
    if (id == PetalID::kNone || id >= PetalID::kNumPetals) return 0;
    if (id == PetalID::kMoon || id == PetalID::kYggdrasil) return 1;
    char const *n = PETAL_DATA[id].name;
    if (n == nullptr) return 0;
    return std::strcmp(n, "Moon") == 0
        || std::strcmp(n, "Yggdrasil") == 0
        || std::strcmp(n, "Golden Leaf") == 0;
}

void compute_mob_drop_weights(MobID::T mob_id, uint8_t mob_rarity,
                              float out[MAX_DROPS_PER_MOB]) {
    for (uint32_t i = 0; i < MAX_DROPS_PER_MOB; ++i) out[i] = 0.f;
    if (mob_id >= MobID::kNumMobs) return;
    auto const &drops = MOB_DATA[mob_id].drops;
    uint32_t n = drops.size();
    if (n > MAX_DROPS_PER_MOB) n = MAX_DROPS_PER_MOB;
    if (n == 0) return;
    if (mob_rarity >= RarityID::kNumRarities) mob_rarity = MOB_DATA[mob_id].rarity;

    // Band d=0 same rarity, d=1 one below, d=2 two below — base shares 20/60/20.
    uint32_t band_n[3] = {};
    uint32_t special_n = 0;
    uint8_t is_special[MAX_DROPS_PER_MOB] = {};
    int band_of[MAX_DROPS_PER_MOB];
    for (uint32_t i = 0; i < n; ++i) {
        band_of[i] = -1;
        PetalID::T id = drops[i];
        if (is_chase_loot_petal(id)) {
            is_special[i] = 1;
            ++special_n;
            continue;
        }
        uint8_t r = PETAL_DATA[id].rarity;
        if (r >= RarityID::kNumRarities || r > mob_rarity) continue;
        int d = (int) mob_rarity - (int) r;
        if (d < 0 || d > 2) continue;
        band_of[i] = d;
        ++band_n[d];
    }

    float special_mass = 0.10f * (float) special_n;
    if (special_mass > 0.90f) special_mass = 0.90f;
    float normal_mass = 1.f - special_mass;

    float band_base[3] = { 0.20f, 0.60f, 0.20f };
    float band_avail = 0.f;
    for (uint32_t d = 0; d < 3; ++d)
        if (band_n[d] > 0) band_avail += band_base[d];
    float band_share[3] = {};
    if (band_avail > 0.f && normal_mass > 0.f) {
        for (uint32_t d = 0; d < 3; ++d) {
            if (band_n[d] == 0) continue;
            band_share[d] = normal_mass * (band_base[d] / band_avail);
        }
    } else if (special_n == 0) {
        // No banded normals and no specials — equal split fallback.
        float eq = 1.f / (float) n;
        for (uint32_t i = 0; i < n; ++i) out[i] = eq;
        return;
    } else {
        // Only specials (or only out-of-band junk): specials take the full pie.
        special_mass = 1.f;
        normal_mass = 0.f;
    }

    float special_each = special_n > 0 ? special_mass / (float) special_n : 0.f;
    float total = 0.f;
    for (uint32_t i = 0; i < n; ++i) {
        if (is_special[i]) out[i] = special_each;
        else if (band_of[i] >= 0)
            out[i] = band_share[band_of[i]] / (float) band_n[band_of[i]];
        else
            out[i] = 0.f;
        total += out[i];
    }
    if (total <= 0.f) {
        float eq = 1.f / (float) n;
        for (uint32_t i = 0; i < n; ++i) out[i] = eq;
        return;
    }
    // Tiny renormalize for float dust.
    for (uint32_t i = 0; i < n; ++i) out[i] /= total;
}

std::array<StaticArray<float, MAX_DROPS_PER_MOB>, MobID::kNumMobs> const MOB_DROP_CHANCES = [](){
    std::array<StaticArray<float, MAX_DROPS_PER_MOB>, MobID::kNumMobs> ret;
    for (MobID::T id = 0; id < MobID::kNumMobs; ++id) {
        float w[MAX_DROPS_PER_MOB] = {};
        compute_mob_drop_weights(id, MOB_DATA[id].rarity, w);
        for (uint32_t i = 0; i < MOB_DATA[id].drops.size(); ++i)
            ret[id].push(w[i]);
    }
    return ret;
}();

uint32_t score_to_pass_level(uint32_t level) {
    // No hard level cap. Soft ≤100 so newbies climb fast; cubic after L100 so
    // even ~150 max-tier kills (zone×spawn×kill_xp_grant_mult=15) stay under L200.
    // L1→2: 75 · L50→51: 810 · L100→101: 1560 · L150→151: ~1.27e5 · L199→200: ~9.7e5
    // cum→100 ≈ 8.0e4 · cum→200 ≈ 2.47e7
    if (level < 1) level = 1;
    if (level <= 100u)
        return 60u + 15u * level;
    uint32_t over = level - 100u;
    uint64_t cube = (uint64_t)over * (uint64_t)over * (uint64_t)over;
    uint64_t total = 1560ull + cube; // 60+15*100
    if (total > UINT32_MAX) return UINT32_MAX;
    return (uint32_t)total;
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

uint32_t score_soft_cap() {
    // Headroom above current grind tops (~Lv180–220); never near UINT32_MAX.
    return level_to_score(300);
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
    // Soft secondary contact DPS (collisions apply every tick @ TPS).
    // Old 2.5*g + 0.015*g^2 → L100≈420 melted soft/mid zones with body alone.
    // Target: L1=25 · L50≈51 · L100≈89 · L150≈141 — petals stay primary damage.
    return BASE_BODY_DAMAGE + 0.4f * (float)gained + 0.0025f * (float)gained * (float)gained;
}
