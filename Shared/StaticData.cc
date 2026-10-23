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
        .health = 12.0,
        .damage = 40.0,
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
        .health = 15.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .secondary_reload = 0.4575, .defend_only = 1 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 30.0,
        .damage = 24.0,
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
        .health = 15.0,
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
        .description = "Poisons and slows targets for a short duration",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .slow_inflict_seconds = 2.05,
            .poison_damage = {
                .damage = 5.0,
                .time = 2.0
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
        .name = "Egg",
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
        .description = "Periodically summons sandstorms to fight for you",
        .health = 10.0,
        .damage = 1.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { 
            .secondary_reload = 2.9,
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
        .description = "Plant sticky webs that snare foes when defending",
        .health = 135.0,
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
        .health = 5.0,
        .damage = 3.0,
        .radius = 12.5,
        .reload = 0.0,
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
        .health = 66.6667,
        .damage = 3.3333,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 10.0,
        .damage = 5.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_range = 84.375, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 12.0,
        .damage = 5.0,
        .radius = 12.5,
        .reload = 0.0,
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
        .health = 10.0,
        .damage = 12.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .damage_reflection = 0.19f }
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
        .health = 19440.0,
        .damage = 6480.0,
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
        .health = 200.0,
        .damage = 120.0,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mass = 220.0 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 6.6667,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_armor = 17.25, .constant_heal = 3.625, .icon_angle = 0.1 }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 15.0,
        .damage = 25.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_body_damage = 28.125, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 20.0,
        .damage = 20.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 0.4313 }
    },
    {
        .name = "Yggdrasil",
        .description = "Unfortunately, its powers are useless here",
        .health = 1200.0,
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
        .health = 45.0,
        .damage = 72.0,
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
        .description = "A sturdy bone that adds petal armor",
        .health = 40.0,
        .damage = 8.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 28.0, .icon_angle = 1.0 }
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
        .health = 450.0,
        .damage = 45.0,
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
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 15.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_reload_factor = 0.9325f, .constant_heal = 2.25, .icon_angle = -1.0 }
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
        .attributes = { .move_speed_factor = 1.1738f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 10.0,
        .damage = 5.0,
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
        .health = 7776.0,
        .damage = 5184.0,
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
        .health = 14.0,
        .damage = 5.5,
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
        .damage = 3.5,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .armor = 8.3125
        }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 7.0,
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
        .health = 315.0,
        .damage = 9.0,
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
        .health = 10.0,
        .damage = 11.5,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 1.5375,
            .poison_damage = { .damage = 4.0, .time = 2.0 }
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 810.0,
        .damage = 540.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_damage_factor = 1.265f
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
                .damage = 269.9973,
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
                .damage = 809.9919,
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
        .health = 4.0,
        .damage = 13.3,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.7916, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 36.0,
        .damage = 120.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.6747, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 108.0,
        .damage = 360.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.6162, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 324.0,
        .damage = 1080.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.5578, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 5.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.5038, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 45.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.4113, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 135.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.365, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 405.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.3187, .defend_only = 1 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 10.0,
        .damage = 8.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_rotation_speed = 0.6688 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 90.0,
        .damage = 72.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 1.1063 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 270.0,
        .damage = 216.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 1.325 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 810.0,
        .damage = 648.0,
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
        .health = 5.0,
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
        .health = 45.0,
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
        .health = 405.0,
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
        .health = 5.5, .damage = 7.0, .radius = 10.0, .reload = 1.625, .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 40.0,
        .damage = 5.0,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 4.1562 }
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
        .health = 15.0, .damage = 11.0, .radius = 15.0, .reload = 3.5, .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 9.0, .damage = 21.0, .radius = 17.0, .reload = 4.75, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 18.0,
        .damage = 6.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 8.9 }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 108.0,
        .damage = 45.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Ancient Seed",
        .description = "Hatches a vine bug from the deep jungle",
        .health = 135.0,
        .damage = 45.0,
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
        .health = 25920.0,
        .damage = 7776.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
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
        .health = 3.3333,
        .damage = 0.5556,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .aura_heal = 8.5, .aura_radius = 210.0, .aura_duration = 16.0, .secondary_reload = 0.6125, .defend_only = 1, .icon_angle = -0.35f }
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
        .health = 32.0,
        .damage = 16.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 65.0,
        .damage = 10.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .damage_reflection = 0.235f, .extra_health = 36.0 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 22.0,
        .damage = 8.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 2.05 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 15.0,
        .damage = 10.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 72.0, .splash_damage = 72.0, .splash_radius = 495.0, .defend_only = 1 }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 40.0,
        .damage = 15.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.9031f }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 28.0,
        .damage = 12.0,
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
        .health = 22.0,
        .damage = 20.0,
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
        .damage = 11.0,
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
        .damage = 6.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 90.0, .defend_only = 1 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 55.0,
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
        .damage = 8.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 6.0, .time = 2.0 } }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 30.0,
        .damage = 19.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .bonus_vs_poisoned = 0.4f }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 16.0,
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
        .health = 120.0,
        .damage = 3.0,
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
        .health = 20.0,
        .damage = 22.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 225.0,
        .damage = 84.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {  .extra_health = 90.0, .splash_damage = 108.0, .splash_radius = 630.0, .burn_damage = { .damage = 6.0, .time = 3.0 } }
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
        .damage = 96.0,
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
        .damage = 60.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.216 }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 90.0,
        .damage = 24.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.255f }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 40.0,
        .damage = 8.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .secondary_reload = 0.3969,
            .aura_heal = 2.9997,
            .aura_radius = 94.9995,
            .aura_duration = 5.575,
            .defend_only = 1, .slow_inflict_seconds = 3.5625 }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 20.0,
        .damage = 17.0,
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
        .health = 160.0,
        .damage = 5.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 1.5012, .extra_health = 594.0 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 25.0,
        .damage = 24.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {  .armor = 216.0, .defend_only = 1, .slow_inflict_seconds = 2.375 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 45.0,
        .damage = 26.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 50.0,
        .damage = 18.0,
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
        .health = 85.0,
        .damage = 30.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.18f, .bonus_vs_slowed = 0.3625f }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 55.0,
        .damage = 8.0,
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
        .health = 40.0,
        .damage = 23.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 5.9994, .time = 5.0004 } }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 50.0,
        .damage = 7.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .move_speed_factor = 1.1813f }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 190.0,
        .damage = 5.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 135.0, .poison_armor = 25.0875 }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 55.0,
        .damage = 48.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.14f, .armor_penetration = 0.3625f }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 60.0,
        .damage = 34.0,
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
        .damage = 8.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.205f, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 70.0,
        .damage = 25.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.7937, .burst_heal = 972.0, .defend_only = 1 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 45.0,
        .damage = 10.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 5.0, .on_hit_heal = 2.7875 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 35.0,
        .damage = 22.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {  .move_speed_factor = 1.2141f, .slow_inflict_seconds = 2.375 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 40.0,
        .damage = 12.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 20.0, .poison_armor = 8.3625 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 50.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 4.0, .extra_health = 405.0, .defend_only = 1 }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 40.0,
        .damage = 40.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 80.0,
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
        .damage = 10.0,
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
        .health = 35.0,
        .damage = 16.0,
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
        .health = 50.0,
        .damage = 42.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 324.0, .slow_inflict_seconds = 2.375 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 40.0,
        .damage = 5.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {  .secondary_reload = 0.7937, .burst_heal = 432.0, .defend_only = 1, .constant_heal = 3.0 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 70.0,
        .damage = 28.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 108.0, .extra_range = 29.9997 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 150.0,
        .damage = 5.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 864.0, .poison_armor = 11.15 }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 55.0,
        .damage = 36.0,
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
        .health = 35.0,
        .damage = 16.0,
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
        .health = 30.0,
        .damage = 4.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .constant_heal = 2.0,
            .poison_damage = { .damage = 2.9997, .time = 2.9997 }
        }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 260.0,
        .damage = 8.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 432.0, .extra_health = 486.0, .taunt_radius = 225.0 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 220.0,
        .damage = 5.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 5.0, .extra_health = 35.0, .defend_only = 1 }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 80.0,
        .damage = 55.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.5375,
            .slow_inflict_seconds = 1.1475,
            .extra_range = 41.9985,
            .radial_pulse_damage = 28.0017,
            .radial_pulse_radius = 140.0004
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 180.0,
        .damage = 65.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_health = 2025.0,
            .burn_damage = { .damage = 8.0, .time = 4.0 }
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 90.0,
        .damage = 20.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.3625,
            .slow_inflict_seconds = 2.7,
            .aura_radius = 120.0015,
            .aura_duration = 10.05,
            .aura_slow_seconds = 1.62,
            .defend_only = 1
        }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 100.0,
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
        .health = 120.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { 
            .secondary_reload = 0.3625,
            .aura_heal = 6.9984,
            .aura_radius = 155.0016,
            .aura_duration = 13.4,
            .defend_only = 1, .constant_heal = 3.0 }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 60.0,
        .damage = 90.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.28f }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 2.9167,
        .damage = 2.0833,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .poison_damage = { .damage = 6.9984, .time = 2.5 },
            .slow_inflict_seconds = 1.0125
        }
    },
    // ---- Ultra / Super / Unique ladders (×2 from Mythic each step) ----
    // Strong Unique Basic (appended after meme kUniqueBasic so craft prefers this id).
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 19440.0,
        .damage = 19440.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 9720.0,
        .damage = 9720.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 4860.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 5832.0,
        .damage = 136080.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 2916.0,
        .damage = 68040.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Stinger",
        .description = "It really hurts, but it's really fragile",
        .health = 1458.0,
        .damage = 34020.0,
        .radius = 7.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 9720.0,
        .damage = 9720.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .secondary_reload = 0.45,
            .burst_heal = 27216.0,
            .defend_only = 1
        }
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 4860.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .secondary_reload = 0.5187,
            .burst_heal = 13608.0,
            .defend_only = 1
        }
    },
    {
        .name = "Rose",
        .description = "Its healing properties are amazing. Not so good at combat though",
        .health = 2430.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .secondary_reload = 0.5875,
            .burst_heal = 6804.0,
            .defend_only = 1
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 19440.0,
        .damage = 15552.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .constant_heal = 1944.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 9720.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .constant_heal = 972.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Leaf",
        .description = "Gathers energy from the sun to passively heal your flower",
        .health = 4860.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .constant_heal = 486.0,
            .icon_angle = -1.0
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 9720.0,
        .damage = 9720.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .poison_damage = {
                .damage = 6479.9352,
                .time = 2.0
            }
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 4860.0,
        .damage = 4860.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .poison_damage = {
                .damage = 3239.9676,
                .time = 2.0
            }
        }
    },
    {
        .name = "Iris",
        .description = "Very poisonous, but takes a while to do its work",
        .health = 2430.0,
        .damage = 2430.0,
        .radius = 7.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .poison_damage = {
                .damage = 1619.9838,
                .time = 2.0
            }
        }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 29160.0,
        .damage = 19440.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 14580.0,
        .damage = 9720.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Wing",
        .description = "Orbits outward, then returns",
        .health = 7290.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .icon_angle = 1.0 }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 2592.0,
        .damage = 8640.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.3825, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 1296.0,
        .damage = 4320.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.4409, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Missile",
        .description = "You can actually shoot this one",
        .health = 648.0,
        .damage = 2160.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.4994, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 3240.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.18, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 1620.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.2263, .defend_only = 1 }
    },
    {
        .name = "Bubble",
        .description = "You can right click to pop it and propel your flower",
        .health = 810.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 1.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.2725, .defend_only = 1 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 6480.0,
        .damage = 5184.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 2.2 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 3240.0,
        .damage = 2592.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 1.9813 }
    },
    {
        .name = "Faster",
        .description = "It's so light it makes your other petals spin faster",
        .health = 1620.0,
        .damage = 1296.0,
        .radius = 7.0,
        .reload = 0.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 1.7625 }
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 97200.0,
        .damage = 9720.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 48600.0,
        .damage = 4860.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Rock",
        .description = "Even more durable, but slower to recharge",
        .health = 24300.0,
        .damage = 2430.0,
        .radius = 12.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 3240.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.28, .defend_only = 1 }
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 1620.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.3325, .defend_only = 1 }
    },
    {
        .name = "Web",
        .description = "Plant sticky webs that snare foes when defending",
        .health = 810.0,
        .damage = 486.0,
        .radius = 10.0,
        .reload = 3.0,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.385, .defend_only = 1 }
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 9720.0,
        .damage = 15552.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 4860.0,
        .damage = 7776.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Fast",
        .description = "Weaker than most petals, but reloads very quickly",
        .health = 2430.0,
        .damage = 3888.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 38880.0,
        .damage = 38880.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 19440.0,
        .damage = 19440.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Heavy",
        .description = "Very resilient and deals more damage, but reloads very slowly",
        .health = 9720.0,
        .damage = 9720.0,
        .radius = 12.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 9720.0,
        .damage = 15552.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 4860.0,
        .damage = 7776.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Twin",
        .description = "Two light petals in one slot",
        .health = 2430.0,
        .damage = 3888.0,
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
        .health = 22.0, .damage = 11.0, .radius = 8.0, .reload = 0.85, .count = 1,
        .rarity = RarityID::kUnusual, .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 52.0,
        .damage = 8.0,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 3.325 }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 18.0,
        .damage = 8.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 7.0, .time = 1.5 } }
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
        .health = 28.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_rotation_speed = 0.384 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 95.0,
        .damage = 12.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 71.0, .armor = 3.3375 }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 40.0,
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
        .damage = 14.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 8.3625, .extra_rotation_speed = 0.375 }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 90.0,
        .damage = 14.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 5.575, .poison_damage = { .damage = 9.0, .time = 2.5 } }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 180.0,
        .damage = 12.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 13.9375, .extra_health = 34.8438 }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 675.0,
        .damage = 135.0,
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
        .health = 540.0,
        .damage = 216.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.725,
            .constant_heal = 4.0,
            .burst_heal = 20.1,
            .extra_rotation_speed = 0.72
        }
    },
    // ---- Monstera ladder (aura_radius rises each rarity; combat ×3 / ×2 like other ladders) ----
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 10.0,
        .damage = 1.6667,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .aura_heal = 10.75, .aura_radius = 245.0, .aura_duration = 18.0, .secondary_reload = 0.5687, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 30.0,
        .damage = 5.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .aura_heal = 13.0, .aura_radius = 280.0, .aura_duration = 20.0, .secondary_reload = 0.525, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 90.0,
        .damage = 15.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .aura_heal = 15.25, .aura_radius = 315.0, .aura_duration = 22.0, .secondary_reload = 0.4812, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 720.0,
        .damage = 120.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .aura_heal = 22.0, .aura_radius = 420.0, .aura_duration = 28.0, .secondary_reload = 0.35, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 360.0,
        .damage = 60.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .aura_heal = 19.75, .aura_radius = 385.0, .aura_duration = 26.0, .secondary_reload = 0.3937, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 180.0,
        .damage = 30.0,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .aura_heal = 17.5, .aura_radius = 350.0, .aura_duration = 24.0, .secondary_reload = 0.4375, .defend_only = 1, .icon_angle = -0.35f }
    },
    // ---- Honey / Meteor (art stubs; AFK/control gameplay wiring later) ----
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 15.0,
        .damage = 10.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 0.3625, .secondary_reload = 0.5625 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 20.0,
        .damage = 40.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mass = 0.75, .aura_duration = 21.0, .secondary_reload = 0.475 }
    },
    // ---- Honey / Meteor rarity ladders ----
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 1.6667,
        .damage = 1.1111,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mass = 0.25, .secondary_reload = 0.65 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 5.0,
        .damage = 3.3333,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mass = 0.3063, .secondary_reload = 0.6063 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 45.0,
        .damage = 30.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mass = 0.4187, .secondary_reload = 0.5188 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 135.0,
        .damage = 90.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mass = 0.475, .secondary_reload = 0.475 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 405.0,
        .damage = 270.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mass = 0.5312, .secondary_reload = 0.4313 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 3240.0,
        .damage = 2160.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mass = 0.7, .secondary_reload = 0.3 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 1620.0,
        .damage = 1080.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mass = 0.6437, .secondary_reload = 0.3438 }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 810.0,
        .damage = 540.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mass = 0.5875, .secondary_reload = 0.3875 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 0.2469,
        .damage = 0.4938,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mass = 0.4, .aura_duration = 12.0, .secondary_reload = 0.65 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 0.7407,
        .damage = 1.4815,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mass = 0.4875, .aura_duration = 14.25, .secondary_reload = 0.6063 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 2.2222,
        .damage = 4.4444,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 0.575, .aura_duration = 16.5, .secondary_reload = 0.5625 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 6.6667,
        .damage = 13.3333,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mass = 0.6625, .aura_duration = 18.75, .secondary_reload = 0.5188 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 60.0,
        .damage = 120.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mass = 0.8375, .aura_duration = 23.25, .secondary_reload = 0.4313 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 480.0,
        .damage = 960.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mass = 1.1, .aura_duration = 30.0, .secondary_reload = 0.3 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 240.0,
        .damage = 480.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mass = 1.0125, .aura_duration = 27.75, .secondary_reload = 0.3438 }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 120.0,
        .damage = 240.0,
        .radius = 18.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mass = 0.925, .aura_duration = 25.5, .secondary_reload = 0.3875 }
    },
    // ---- Golden Leaf ladder (CD reduction; stacks multiplicatively) ----
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 45.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_reload_factor = 0.9137f, .constant_heal = 2.875, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 135.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_reload_factor = 0.895f, .constant_heal = 3.5, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 405.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_reload_factor = 0.8762f, .constant_heal = 4.125, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 810.0,
        .damage = 540.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_reload_factor = 0.8575f, .constant_heal = 4.75, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 1620.0,
        .damage = 1080.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_reload_factor = 0.8387f, .constant_heal = 5.375, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 3240.0,
        .damage = 2160.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_reload_factor = 0.82f, .constant_heal = 6.0, .icon_angle = -1.0 }
    },
    // ---- Auto-filled rarity ladders (gen_petal_ladders.py) ----
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 240.0,
        .damage = 165.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 2.2969,
            .slow_inflict_seconds = 1.2856,
            .extra_range = 125.9955,
            .radial_pulse_damage = 84.0051,
            .radial_pulse_radius = 420.0012
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 480.0,
        .damage = 330.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 2.0562,
            .slow_inflict_seconds = 1.4238,
            .extra_range = 251.991,
            .radial_pulse_damage = 168.0102,
            .radial_pulse_radius = 840.0024
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 960.0,
        .damage = 660.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.8156,
            .slow_inflict_seconds = 1.5619,
            .extra_range = 503.982,
            .radial_pulse_damage = 336.0204,
            .radial_pulse_radius = 1680.0048
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 1920.0,
        .damage = 1320.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.575,
            .slow_inflict_seconds = 1.7,
            .extra_range = 1007.964,
            .radial_pulse_damage = 672.0408,
            .radial_pulse_radius = 3360.0096
        }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 24.0,
        .damage = 15.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .move_speed_factor = 1.2275f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 72.0,
        .damage = 45.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .move_speed_factor = 1.2812f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 216.0,
        .damage = 135.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .move_speed_factor = 1.335f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 648.0,
        .damage = 405.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .move_speed_factor = 1.3887f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 1296.0,
        .damage = 810.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .move_speed_factor = 1.4425f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 2592.0,
        .damage = 1620.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .move_speed_factor = 1.4963f, .equipment = EquipmentFlags::kAccelerator }
    },
    {
        .name = "Accelerator",
        .description = "A light boost for long runs — faster travel without rocketing away",
        .health = 5184.0,
        .damage = 3240.0,
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
        .damage = 24.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 21.0, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 162.0,
        .damage = 72.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 63.0, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 486.0,
        .damage = 216.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 189.0, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 1458.0,
        .damage = 648.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 567.0, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 2916.0,
        .damage = 1296.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 1134.0, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 5832.0,
        .damage = 2592.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 2268.0, .time = 1.5 } }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 11664.0,
        .damage = 5184.0,
        .radius = 9.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 4536.0, .time = 1.5 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 48.0,
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
        .health = 144.0,
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
        .health = 432.0,
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
        .health = 1296.0,
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
        .health = 2592.0,
        .damage = 1458.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 486.0, .time = 4.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 5184.0,
        .damage = 2916.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 972.0, .time = 4.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 10368.0,
        .damage = 5832.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 1944.0, .time = 4.0 } }
    },
    {
        .name = "Ancient Coin",
        .description = "Heavy golden coins with a sharp edge",
        .health = 30.0,
        .damage = 48.0,
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
        .health = 90.0,
        .damage = 144.0,
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
        .health = 270.0,
        .damage = 432.0,
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
        .health = 810.0,
        .damage = 1296.0,
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
        .health = 1620.0,
        .damage = 2592.0,
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
        .health = 3240.0,
        .damage = 5184.0,
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
        .health = 6480.0,
        .damage = 10368.0,
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
        .damage = 135.0,
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
        .damage = 405.0,
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
        .health = 2430.0,
        .damage = 810.0,
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
        .health = 4860.0,
        .damage = 1620.0,
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
        .health = 9720.0,
        .damage = 3240.0,
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
        .health = 135.0,
        .damage = 30.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 15.0, .on_hit_heal = 3.35 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 405.0,
        .damage = 90.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 45.0, .on_hit_heal = 3.9125 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 810.0,
        .damage = 180.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 90.0, .on_hit_heal = 4.475 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 1620.0,
        .damage = 360.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 180.0, .on_hit_heal = 5.0375 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 3240.0,
        .damage = 720.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 360.0, .on_hit_heal = 5.6 }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 90.0,
        .damage = 57.0,
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
        .damage = 171.0,
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
        .damage = 513.0,
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
        .damage = 1539.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .bonus_vs_poisoned = 0.6f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 4860.0,
        .damage = 3078.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .bonus_vs_poisoned = 0.65f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 9720.0,
        .damage = 6156.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .bonus_vs_poisoned = 0.7f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 19440.0,
        .damage = 12312.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .bonus_vs_poisoned = 0.75f }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 300.0,
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
        .health = 600.0,
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
        .health = 1200.0,
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
        .health = 2400.0,
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
        .health = 15.0,
        .damage = 9.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kMythic,
        .attributes = { .vision_factor = 0.5687f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 30.0,
        .damage = 18.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kUltra,
        .attributes = { .vision_factor = 0.5125f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 60.0,
        .damage = 36.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kSuper,
        .attributes = { .vision_factor = 0.4563f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 120.0,
        .damage = 72.0,
        .radius = 12.5,
        .reload = 0.0,
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
        .health = 2916.0,
        .damage = 2916.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 486.0, .time = 2.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 5832.0,
        .damage = 5832.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 972.0, .time = 2.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 11664.0,
        .damage = 11664.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 1944.0, .time = 2.0 } }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 120.0,
        .damage = 24.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 33.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 240.0,
        .damage = 48.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 38.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 480.0,
        .damage = 96.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 43.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 960.0,
        .damage = 192.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 48.0, .icon_angle = 1.0 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 105.0,
        .damage = 12.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 7.5, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 315.0,
        .damage = 36.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 22.5, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 945.0,
        .damage = 108.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 67.5, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 1890.0,
        .damage = 216.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 135.0, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 3780.0,
        .damage = 432.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 270.0, .defend_only = 1 }
    },
    {
        .name = "Brood Pearl",
        .description = "A smooth brood egg that slowly restores your flower.",
        .health = 7560.0,
        .damage = 864.0,
        .radius = 11.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 540.0, .defend_only = 1 }
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
        .health = 9720.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .extra_health = 9720.0, .damage_reflection = 0.31f }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 19440.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .extra_health = 19440.0, .damage_reflection = 0.345f }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 38880.0,
        .damage = 9720.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .extra_health = 38880.0, .damage_reflection = 0.38f }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 8.75,
        .damage = 6.25,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .poison_damage = { .damage = 20.9952, .time = 2.5 },
            .slow_inflict_seconds = 1.1344
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 17.5,
        .damage = 12.5,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .poison_damage = { .damage = 41.9904, .time = 2.5 },
            .slow_inflict_seconds = 1.2563
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 35.0,
        .damage = 25.0,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .poison_damage = { .damage = 83.9808, .time = 2.5 },
            .slow_inflict_seconds = 1.3781
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 70.0,
        .damage = 50.0,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .poison_damage = { .damage = 167.9616, .time = 2.5 },
            .slow_inflict_seconds = 1.5
        }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 66.0,
        .damage = 33.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 198.0,
        .damage = 99.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 594.0,
        .damage = 297.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 1782.0,
        .damage = 891.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 3564.0,
        .damage = 1782.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 7128.0,
        .damage = 3564.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chitin Needle",
        .description = "A slender chitin needle that strikes rapidly and slips through a small amount of armor.",
        .health = 14256.0,
        .damage = 7128.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor_penetration = 0.08f }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 270.0,
        .damage = 60.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.3281,
            .slow_inflict_seconds = 3.025,
            .aura_radius = 360.0045,
            .aura_duration = 11.7375,
            .aura_slow_seconds = 1.815,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 540.0,
        .damage = 120.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.2937,
            .slow_inflict_seconds = 3.35,
            .aura_radius = 720.009,
            .aura_duration = 13.425,
            .aura_slow_seconds = 2.01,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 1080.0,
        .damage = 240.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.2594,
            .slow_inflict_seconds = 3.675,
            .aura_radius = 1440.018,
            .aura_duration = 15.1125,
            .aura_slow_seconds = 2.205,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 2160.0,
        .damage = 480.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.225,
            .slow_inflict_seconds = 4.0,
            .aura_radius = 2880.036,
            .aura_duration = 16.8,
            .aura_slow_seconds = 2.4,
            .defend_only = 1
        }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 150.0,
        .damage = 21.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .move_speed_factor = 1.215f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 450.0,
        .damage = 63.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .move_speed_factor = 1.2488f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 900.0,
        .damage = 126.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .move_speed_factor = 1.2825f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 1800.0,
        .damage = 252.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .move_speed_factor = 1.3163f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 3600.0,
        .damage = 504.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .move_speed_factor = 1.35f }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 1620.0,
        .damage = 648.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.6562,
            .constant_heal = 12.0,
            .burst_heal = 23.475,
            .extra_rotation_speed = 0.84
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 3240.0,
        .damage = 1296.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5875,
            .constant_heal = 24.0,
            .burst_heal = 26.85,
            .extra_rotation_speed = 0.96
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 6480.0,
        .damage = 2592.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.5187,
            .constant_heal = 48.0,
            .burst_heal = 30.225,
            .extra_rotation_speed = 1.08
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 12960.0,
        .damage = 5184.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.45,
            .constant_heal = 96.0,
            .burst_heal = 33.6,
            .extra_rotation_speed = 1.2
        }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 120.0,
        .damage = 36.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 20.0, .poison_armor = 10.05 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 360.0,
        .damage = 108.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 20.0, .poison_armor = 11.7375 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 720.0,
        .damage = 216.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 20.0, .poison_armor = 13.425 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 1440.0,
        .damage = 432.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 20.0, .poison_armor = 15.1125 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 2880.0,
        .damage = 864.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 20.0, .poison_armor = 16.8 }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 1350.0,
        .damage = 135.0,
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
        .health = 4050.0,
        .damage = 405.0,
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
        .health = 8100.0,
        .damage = 810.0,
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
        .health = 16200.0,
        .damage = 1620.0,
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
        .health = 32400.0,
        .damage = 3240.0,
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
        .health = 120.0,
        .damage = 69.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 17.9982, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 360.0,
        .damage = 207.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 53.9946, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 720.0,
        .damage = 414.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 107.9892, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 1440.0,
        .damage = 828.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 215.9784, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 2880.0,
        .damage = 1656.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 431.9568, .time = 5.0004 } }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 195.0,
        .damage = 30.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .damage_reflection = 0.27f, .extra_health = 108.0 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 585.0,
        .damage = 90.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.305f, .extra_health = 324.0 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 1755.0,
        .damage = 270.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.34f, .extra_health = 972.0 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 5265.0,
        .damage = 810.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.375f, .extra_health = 2916.0 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 10530.0,
        .damage = 1620.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.41f, .extra_health = 5832.0 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 21060.0,
        .damage = 3240.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.445f, .extra_health = 11664.0 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 42120.0,
        .damage = 6480.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.48f, .extra_health = 23328.0 }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 45.0,
        .damage = 75.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_body_damage = 33.5, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 135.0,
        .damage = 225.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_body_damage = 38.875, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 270.0,
        .damage = 450.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_body_damage = 44.25, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 540.0,
        .damage = 900.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_body_damage = 49.625, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 1080.0,
        .damage = 1800.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_body_damage = 55.0, .equipment = EquipmentFlags::kCutter }
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
        .health = 2430.0,
        .damage = 2430.0,
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
        .health = 4860.0,
        .damage = 4860.0,
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
        .health = 9720.0,
        .damage = 9720.0,
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
        .health = 255.0,
        .damage = 90.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.2f, .bonus_vs_slowed = 0.4f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 765.0,
        .damage = 270.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.22f, .bonus_vs_slowed = 0.4375f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 1530.0,
        .damage = 540.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.24f, .bonus_vs_slowed = 0.475f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 3060.0,
        .damage = 1080.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.26f, .bonus_vs_slowed = 0.5125f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 6120.0,
        .damage = 2160.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.28f, .bonus_vs_slowed = 0.55f }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 480.0,
        .damage = 15.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 4.5036, .extra_health = 1782.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 1440.0,
        .damage = 45.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 13.5108, .extra_health = 5346.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 2880.0,
        .damage = 90.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 27.0216, .extra_health = 10692.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 5760.0,
        .damage = 180.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 54.0432, .extra_health = 21384.0 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 11520.0,
        .damage = 360.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 108.0864, .extra_health = 42768.0 }
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
        .health = 9720.0,
        .damage = 486.0,
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
        .health = 19440.0,
        .damage = 972.0,
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
        .health = 38880.0,
        .damage = 1944.0,
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
        .damage = 15.0,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 5.5625 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 360.0,
        .damage = 45.0,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 6.9688 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 1080.0,
        .damage = 135.0,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 8.375 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 3240.0,
        .damage = 405.0,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 9.7812 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 6480.0,
        .damage = 810.0,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 11.1875 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 12960.0,
        .damage = 1620.0,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 12.5938 }
    },
    {
        .name = "Emerald Shell",
        .description = "A sturdy emerald carapace",
        .health = 25920.0,
        .damage = 3240.0,
        .radius = 17.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 14.0 }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 60.0,
        .damage = 66.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .burn_damage = { .damage = 12.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 180.0,
        .damage = 198.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .burn_damage = { .damage = 36.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 540.0,
        .damage = 594.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .burn_damage = { .damage = 108.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 1620.0,
        .damage = 1782.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .burn_damage = { .damage = 324.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 3240.0,
        .damage = 3564.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .burn_damage = { .damage = 648.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 6480.0,
        .damage = 7128.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .burn_damage = { .damage = 1296.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 12960.0,
        .damage = 14256.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .burn_damage = { .damage = 2592.0, .time = 3.0 } }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 180.0,
        .damage = 102.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.45, .extra_range = 24.0003 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 540.0,
        .damage = 306.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.525, .extra_range = 72.0009 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 1080.0,
        .damage = 612.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.6, .extra_range = 144.0018 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 2160.0,
        .damage = 1224.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.675, .extra_range = 288.0036 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 4320.0,
        .damage = 2448.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.75, .extra_range = 576.0072 }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 165.0,
        .damage = 144.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.16f, .armor_penetration = 0.4f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 495.0,
        .damage = 432.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.18f, .armor_penetration = 0.4375f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 990.0,
        .damage = 864.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.2f, .armor_penetration = 0.475f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 1980.0,
        .damage = 1728.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.22f, .armor_penetration = 0.5125f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 3960.0,
        .damage = 3456.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.24f, .armor_penetration = 0.55f }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 75.0,
        .damage = 72.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {  .armor = 648.0, .defend_only = 1, .slow_inflict_seconds = 2.7 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 225.0,
        .damage = 216.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {  .armor = 1944.0, .defend_only = 1, .slow_inflict_seconds = 3.025 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 450.0,
        .damage = 432.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {  .armor = 3888.0, .defend_only = 1, .slow_inflict_seconds = 3.35 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 900.0,
        .damage = 864.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {  .armor = 7776.0, .defend_only = 1, .slow_inflict_seconds = 3.675 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 1800.0,
        .damage = 1728.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {  .armor = 15552.0, .defend_only = 1, .slow_inflict_seconds = 4.0 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 450.0,
        .damage = 15.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 2592.0, .poison_armor = 13.4 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 1350.0,
        .damage = 45.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 7776.0, .poison_armor = 15.65 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 2700.0,
        .damage = 90.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 15552.0, .poison_armor = 17.9 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 5400.0,
        .damage = 180.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 31104.0, .poison_armor = 20.15 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 10800.0,
        .damage = 360.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 62208.0, .poison_armor = 22.4 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 150.0,
        .damage = 126.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 972.0, .slow_inflict_seconds = 2.7 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 450.0,
        .damage = 378.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 2916.0, .slow_inflict_seconds = 3.025 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 900.0,
        .damage = 756.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 5832.0, .slow_inflict_seconds = 3.35 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 1800.0,
        .damage = 1512.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 11664.0, .slow_inflict_seconds = 3.675 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 3600.0,
        .damage = 3024.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 23328.0, .slow_inflict_seconds = 4.0 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 135.0,
        .damage = 78.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 405.0,
        .damage = 234.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 810.0,
        .damage = 468.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 1620.0,
        .damage = 936.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 3240.0,
        .damage = 1872.0,
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
        .damage = 48.0,
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
        .health = 315.0,
        .damage = 144.0,
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
        .health = 630.0,
        .damage = 288.0,
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
        .health = 1260.0,
        .damage = 576.0,
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
        .health = 2520.0,
        .damage = 1152.0,
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
        .health = 945.0,
        .damage = 27.0,
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
        .health = 2835.0,
        .damage = 81.0,
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
        .health = 5670.0,
        .damage = 162.0,
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
        .health = 11340.0,
        .damage = 324.0,
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
        .health = 22680.0,
        .damage = 648.0,
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
        .health = 2916.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 972.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 5832.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 1944.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 11664.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 3888.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 660.0,
        .damage = 15.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 15.0, .extra_health = 105.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 1320.0,
        .damage = 30.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 30.0, .extra_health = 210.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 2640.0,
        .damage = 60.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 60.0, .extra_health = 420.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 5280.0,
        .damage = 120.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 120.0, .extra_health = 840.0, .defend_only = 1 }
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 200.0,
        .damage = 10.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 600.0,
        .damage = 30.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 1200.0,
        .damage = 60.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 2400.0,
        .damage = 120.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 4800.0,
        .damage = 240.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 420.0,
        .damage = 42.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 10.05, .extra_rotation_speed = 0.45 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 1260.0,
        .damage = 126.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 11.7375, .extra_rotation_speed = 0.525 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 2520.0,
        .damage = 252.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 13.425, .extra_rotation_speed = 0.6 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 5040.0,
        .damage = 504.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 15.1125, .extra_rotation_speed = 0.675 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 10080.0,
        .damage = 1008.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 16.8, .extra_rotation_speed = 0.75 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 210.0,
        .damage = 75.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 0.725, .burst_heal = 2916.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 630.0,
        .damage = 225.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.6562, .burst_heal = 8748.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 1260.0,
        .damage = 450.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.5875, .burst_heal = 17496.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 2520.0,
        .damage = 900.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.5187, .burst_heal = 34992.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 5040.0,
        .damage = 1800.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.45, .burst_heal = 69984.0, .defend_only = 1 }
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
        .health = 3888.0,
        .damage = 2916.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.5875, .burst_heal = 9720.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 7776.0,
        .damage = 5832.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.5187, .burst_heal = 19440.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 15552.0,
        .damage = 11664.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 0.45, .burst_heal = 38880.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 120.0,
        .damage = 15.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {  .secondary_reload = 0.725, .burst_heal = 1296.0, .defend_only = 1, .constant_heal = 9.0 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 360.0,
        .damage = 45.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {  .secondary_reload = 0.6562, .burst_heal = 3888.0, .defend_only = 1, .constant_heal = 27.0 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 720.0,
        .damage = 90.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {  .secondary_reload = 0.5875, .burst_heal = 7776.0, .defend_only = 1, .constant_heal = 54.0 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 1440.0,
        .damage = 180.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {  .secondary_reload = 0.5187, .burst_heal = 15552.0, .defend_only = 1, .constant_heal = 108.0 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 2880.0,
        .damage = 360.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {  .secondary_reload = 0.45, .burst_heal = 31104.0, .defend_only = 1, .constant_heal = 216.0 }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 90.0,
        .damage = 12.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .constant_heal = 6.0,
            .poison_damage = { .damage = 8.9991, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 270.0,
        .damage = 36.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .constant_heal = 18.0,
            .poison_damage = { .damage = 26.9973, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 540.0,
        .damage = 72.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .constant_heal = 36.0,
            .poison_damage = { .damage = 53.9946, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 1080.0,
        .damage = 144.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .constant_heal = 72.0,
            .poison_damage = { .damage = 107.9892, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 2160.0,
        .damage = 288.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .constant_heal = 144.0,
            .poison_damage = { .damage = 215.9784, .time = 2.9997 }
        }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 330.0,
        .damage = 18.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 270.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 990.0,
        .damage = 54.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 810.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 2970.0,
        .damage = 162.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 2430.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 5940.0,
        .damage = 324.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 4860.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 11880.0,
        .damage = 648.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 9720.0, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 23760.0,
        .damage = 1296.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 19440.0, .defend_only = 1 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 66.0,
        .damage = 60.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 1.5375 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 198.0,
        .damage = 180.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 1.7812 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 594.0,
        .damage = 540.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 2.025 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 1782.0,
        .damage = 1620.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 2.2687 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 3564.0,
        .damage = 3240.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 2.5125 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 7128.0,
        .damage = 6480.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 2.7563 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 14256.0,
        .damage = 12960.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 3.0 }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 150.0,
        .damage = 54.0,
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
        .health = 450.0,
        .damage = 162.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.9688,
            .slow_inflict_seconds = 0.6806,
            .extra_range = 224.9937,
            .radial_pulse_damage = 162.0081,
            .radial_pulse_radius = 990.0063
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 900.0,
        .damage = 324.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.7625,
            .slow_inflict_seconds = 0.7537,
            .extra_range = 449.9874,
            .radial_pulse_damage = 324.0162,
            .radial_pulse_radius = 1980.0126
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 1800.0,
        .damage = 648.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.5563,
            .slow_inflict_seconds = 0.8269,
            .extra_range = 899.9748,
            .radial_pulse_damage = 648.0324,
            .radial_pulse_radius = 3960.0252
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 3600.0,
        .damage = 1296.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.35,
            .slow_inflict_seconds = 0.9,
            .extra_range = 1799.9496,
            .radial_pulse_damage = 1296.0648,
            .radial_pulse_radius = 7920.0504
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 540.0,
        .damage = 195.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_health = 6075.0,
            .burn_damage = { .damage = 24.0, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 1080.0,
        .damage = 390.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_health = 12150.0,
            .burn_damage = { .damage = 48.0, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 2160.0,
        .damage = 780.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_health = 24300.0,
            .burn_damage = { .damage = 96.0, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 4320.0,
        .damage = 1560.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_health = 48600.0,
            .burn_damage = { .damage = 192.0, .time = 4.0 }
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
        .attributes = { .armor = 2.4937 }
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
        .attributes = { .armor = 3.3375 }
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
        .attributes = { .armor = 4.1812 }
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
        .attributes = { .armor = 5.025 }
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
        .attributes = { .armor = 5.8687 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 17496.0,
        .damage = 2673.0,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 6.7125 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 34992.0,
        .damage = 5346.0,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 7.5562 }
    },
    {
        .name = "Iron Guard",
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 69984.0,
        .damage = 10692.0,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 8.4 }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 120.0,
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
        .health = 360.0,
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
        .health = 1080.0,
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
        .health = 2160.0,
        .damage = 972.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.3225f }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 4320.0,
        .damage = 1944.0,
        .radius = 10.0,
        .reload = 1.1,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.3512f }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 8640.0,
        .damage = 3888.0,
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
        .health = 2673.0,
        .damage = 3402.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 5346.0,
        .damage = 6804.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "Jungle Needle",
        .description = "A thin canopy needle that strikes in a clump",
        .health = 10692.0,
        .damage = 13608.0,
        .radius = 10.0,
        .reload = 1.625,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 8 }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 2430.0,
        .damage = 1620.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_damage_factor = 1.2937f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 4860.0,
        .damage = 3240.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_damage_factor = 1.3225f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 9720.0,
        .damage = 6480.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_damage_factor = 1.3512f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 19440.0,
        .damage = 12960.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_damage_factor = 1.38f
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
        .health = 12150.0,
        .damage = 5832.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 24300.0,
        .damage = 11664.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 48600.0,
        .damage = 23328.0,
        .radius = 12.0,
        .reload = 1.6,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 10.0 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 150.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 12.0, .extra_health = 1215.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 450.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 36.0, .extra_health = 3645.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 900.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 72.0, .extra_health = 7290.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 1800.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 144.0, .extra_health = 14580.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 3600.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 288.0, .extra_health = 29160.0, .defend_only = 1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 20.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_armor = 21.0, .constant_heal = 4.5, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 60.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_armor = 24.75, .constant_heal = 5.375, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 120.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_armor = 28.5, .constant_heal = 6.25, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 240.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_armor = 32.25, .constant_heal = 7.125, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 480.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_armor = 36.0, .constant_heal = 8.0, .icon_angle = 0.1 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 675.0,
        .damage = 252.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {  .extra_health = 270.0, .splash_damage = 324.0, .splash_radius = 1890.0, .burn_damage = { .damage = 18.0, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 2025.0,
        .damage = 756.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {  .extra_health = 810.0, .splash_damage = 972.0, .splash_radius = 5670.0, .burn_damage = { .damage = 54.0, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 6075.0,
        .damage = 2268.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {  .extra_health = 2430.0, .splash_damage = 2916.0, .splash_radius = 17010.0, .burn_damage = { .damage = 162.0, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 12150.0,
        .damage = 4536.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {  .extra_health = 4860.0, .splash_damage = 5832.0, .splash_radius = 34020.0, .burn_damage = { .damage = 324.0, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 24300.0,
        .damage = 9072.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {  .extra_health = 9720.0, .splash_damage = 11664.0, .splash_radius = 68040.0, .burn_damage = { .damage = 648.0, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 48600.0,
        .damage = 18144.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {  .extra_health = 19440.0, .splash_damage = 23328.0, .splash_radius = 136080.0, .burn_damage = { .damage = 1296.0, .time = 3.0 } }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 30.0,
        .damage = 15.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .magnet_radius = 925.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 90.0,
        .damage = 45.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .magnet_radius = 1137.5, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 270.0,
        .damage = 135.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .magnet_radius = 1350.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 810.0,
        .damage = 405.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .magnet_radius = 1562.5, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 1620.0,
        .damage = 810.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .magnet_radius = 1775.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 3240.0,
        .damage = 1620.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .magnet_radius = 1987.5, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 6480.0,
        .damage = 3240.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .magnet_radius = 2200.0, .equipment = EquipmentFlags::kMagnet }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 27.0,
        .damage = 63.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 81.0,
        .damage = 189.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 162.0,
        .damage = 378.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 324.0,
        .damage = 756.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mantis Claw",
        .description = "A fragile scythe-like claw that delivers a devastating finishing strike against weakened enemies.",
        .health = 648.0,
        .damage = 1512.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .execute_threshold = 0.20f, .execute_factor = 1.30f }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 720.0,
        .damage = 180.0,
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
        .damage = 540.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.324 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 6480.0,
        .damage = 1620.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.378 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 12960.0,
        .damage = 3240.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.432 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 25920.0,
        .damage = 6480.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.486 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 51840.0,
        .damage = 12960.0,
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
        .damage = 48.0,
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
        .health = 315.0,
        .damage = 144.0,
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
        .health = 630.0,
        .damage = 288.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_rotation_speed = 0.336,
            .poison_damage = { .damage = 53.9946, .time = 2.0007 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 1260.0,
        .damage = 576.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_rotation_speed = 0.378,
            .poison_damage = { .damage = 107.9892, .time = 2.0007 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 2520.0,
        .damage = 1152.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_rotation_speed = 0.42,
            .poison_damage = { .damage = 215.9784, .time = 2.0007 }
        }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 120.0,
        .damage = 120.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 360.0,
        .damage = 360.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 720.0,
        .damage = 720.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 1440.0,
        .damage = 1440.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 2880.0,
        .damage = 2880.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 24.0,
        .damage = 10.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kUltra,
        .attributes = { .vision_factor = 0.2725f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 48.0,
        .damage = 20.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kSuper,
        .attributes = { .vision_factor = 0.2263f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 96.0,
        .damage = 40.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kUnique,
        .attributes = { .vision_factor = 0.18f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 216.0,
        .damage = 306.0,
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
        .damage = 918.0,
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
        .damage = 2754.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor_penetration = 0.6562f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 3888.0,
        .damage = 5508.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor_penetration = 0.6875f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 7776.0,
        .damage = 11016.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor_penetration = 0.7188f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 15552.0,
        .damage = 22032.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor_penetration = 0.75f }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 720.0,
        .damage = 24.0,
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
        .damage = 72.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.275f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 4320.0,
        .damage = 144.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.31f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 8640.0,
        .damage = 288.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.345f, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 17280.0,
        .damage = 576.0,
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
        .health = 2430.0,
        .damage = 3888.0,
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
        .health = 4860.0,
        .damage = 7776.0,
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
        .health = 9720.0,
        .damage = 15552.0,
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
        .health = 210.0,
        .damage = 84.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 324.0, .extra_range = 89.9991 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 630.0,
        .damage = 252.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 972.0, .extra_range = 269.9973 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 1260.0,
        .damage = 504.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 1944.0, .extra_range = 539.9946 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 2520.0,
        .damage = 1008.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 3888.0, .extra_range = 1079.9892 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 5040.0,
        .damage = 2016.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 7776.0, .extra_range = 2159.9784 }
    },
    {
        .name = "Phantom Eye",
        .description = "Amplifies your petal damage while equipped",
        .health = 21.0,
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
        .health = 63.0,
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
        .health = 189.0,
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
        .health = 378.0,
        .damage = 729.0,
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
        .health = 756.0,
        .damage = 1458.0,
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
        .health = 1512.0,
        .damage = 2916.0,
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
        .health = 84.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 0.48 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 252.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.576 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 756.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.672 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 1512.0,
        .damage = 540.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.768 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 3024.0,
        .damage = 1080.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.864 }
    },
    {
        .name = "Pheromone Star",
        .description = "Colony pheromones that spin your petals faster.",
        .health = 6048.0,
        .damage = 2160.0,
        .radius = 10.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.96 }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 30.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .slow_inflict_seconds = 2.375,
            .poison_damage = {
                .damage = 15.0,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 90.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .slow_inflict_seconds = 2.7,
            .poison_damage = {
                .damage = 45.0,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 270.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .slow_inflict_seconds = 3.025,
            .poison_damage = {
                .damage = 135.0,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 540.0,
        .damage = 540.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .slow_inflict_seconds = 3.35,
            .poison_damage = {
                .damage = 270.0,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 1080.0,
        .damage = 1080.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .slow_inflict_seconds = 3.675,
            .poison_damage = {
                .damage = 540.0,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Pincer",
        .description = "Poisons and slows targets for a short duration",
        .health = 2160.0,
        .damage = 2160.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .slow_inflict_seconds = 4.0,
            .poison_damage = {
                .damage = 1080.0,
                .time = 2.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 165.0,
        .damage = 24.0,
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
        .health = 495.0,
        .damage = 72.0,
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
        .health = 990.0,
        .damage = 144.0,
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
        .health = 1980.0,
        .damage = 288.0,
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
        .health = 3960.0,
        .damage = 576.0,
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
        .health = 5832.0,
        .damage = 1944.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 1944.0, .time = 4.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 11664.0,
        .damage = 3888.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 3888.0, .time = 4.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 23328.0,
        .damage = 7776.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 9.0, .poison_damage = { .damage = 7776.0, .time = 4.0 } }
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
        .health = 2430.0,
        .damage = 3888.0,
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
        .health = 4860.0,
        .damage = 7776.0,
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
        .health = 9720.0,
        .damage = 15552.0,
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
        .health = 120.0,
        .damage = 24.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { 
            .secondary_reload = 0.3625,
            .aura_heal = 8.9991,
            .aura_radius = 284.9985,
            .aura_duration = 6.7,
            .defend_only = 1, .slow_inflict_seconds = 4.05 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 360.0,
        .damage = 72.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { 
            .secondary_reload = 0.3281,
            .aura_heal = 26.9973,
            .aura_radius = 854.9955,
            .aura_duration = 7.825,
            .defend_only = 1, .slow_inflict_seconds = 4.5375 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 720.0,
        .damage = 144.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .secondary_reload = 0.2937,
            .aura_heal = 53.9946,
            .aura_radius = 1709.991,
            .aura_duration = 8.95,
            .defend_only = 1, .slow_inflict_seconds = 5.025 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 1440.0,
        .damage = 288.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .secondary_reload = 0.2594,
            .aura_heal = 107.9892,
            .aura_radius = 3419.982,
            .aura_duration = 10.075,
            .defend_only = 1, .slow_inflict_seconds = 5.5125 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 2880.0,
        .damage = 576.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .secondary_reload = 0.225,
            .aura_heal = 215.9784,
            .aura_radius = 6839.964,
            .aura_duration = 11.2,
            .defend_only = 1, .slow_inflict_seconds = 6.0 }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 2025.0,
        .damage = 405.0,
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
        .health = 4050.0,
        .damage = 810.0,
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
        .health = 8100.0,
        .damage = 1620.0,
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
        .health = 16200.0,
        .damage = 3240.0,
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
        .health = 66.0,
        .damage = 24.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 2.375 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 198.0,
        .damage = 72.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 2.7 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 594.0,
        .damage = 216.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 3.025 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 1188.0,
        .damage = 432.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 3.35 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 2376.0,
        .damage = 864.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 3.675 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 4752.0,
        .damage = 1728.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 4.0 }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 135.0,
        .damage = 216.0,
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
        .health = 405.0,
        .damage = 648.0,
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
        .health = 810.0,
        .damage = 1296.0,
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
        .health = 1620.0,
        .damage = 2592.0,
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
        .health = 3240.0,
        .damage = 5184.0,
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
        .health = 360.0,
        .damage = 24.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 18.0, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 1080.0,
        .damage = 72.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 54.0, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 3240.0,
        .damage = 216.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 162.0, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 6480.0,
        .damage = 432.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 324.0, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 12960.0,
        .damage = 864.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 648.0, .time = 2.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 25920.0,
        .damage = 1728.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 1296.0, .time = 2.0 } }
    },
    {
        .name = "Royal Banner",
        .description = "Inspires faster petal spin while equipped",
        .health = 42.0,
        .damage = 16.5,
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
        .health = 126.0,
        .damage = 49.5,
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
        .health = 378.0,
        .damage = 148.5,
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
        .health = 1134.0,
        .damage = 445.5,
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
        .health = 2268.0,
        .damage = 891.0,
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
        .health = 4536.0,
        .damage = 1782.0,
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
        .health = 9072.0,
        .damage = 3564.0,
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
        .health = 540.0,
        .damage = 36.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 16.75, .extra_health = 41.875 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 1620.0,
        .damage = 108.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 19.5625, .extra_health = 48.9062 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 3240.0,
        .damage = 216.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 22.375, .extra_health = 55.9375 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 6480.0,
        .damage = 432.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 25.1875, .extra_health = 62.9688 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 12960.0,
        .damage = 864.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 28.0, .extra_health = 70.0 }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 27.0,
        .damage = 25.5,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 24.0, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 81.0,
        .damage = 76.5,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 72.0, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 243.0,
        .damage = 229.5,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 216.0, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 486.0,
        .damage = 459.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 432.0, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 972.0,
        .damage = 918.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 864.0, .time = 3.0 }
        }
    },
    {
        .name = "Royal Rose",
        .description = "A regal rose that leaves lingering poison",
        .health = 1944.0,
        .damage = 1836.0,
        .radius = 14.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 10,
            .poison_damage = { .damage = 1728.0, .time = 3.0 }
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 25.5,
        .damage = 60.0,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_range = 45.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 76.5,
        .damage = 180.0,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_range = 45.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 229.5,
        .damage = 540.0,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_range = 45.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 459.0,
        .damage = 1080.0,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_range = 45.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 918.0,
        .damage = 2160.0,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_range = 45.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 1836.0,
        .damage = 4320.0,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_range = 45.f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 165.0,
        .damage = 108.0,
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
        .health = 495.0,
        .damage = 324.0,
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
        .health = 990.0,
        .damage = 648.0,
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
        .health = 1980.0,
        .damage = 1296.0,
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
        .health = 3960.0,
        .damage = 2592.0,
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
        .health = 2673.0,
        .damage = 3159.0,
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
        .health = 5346.0,
        .damage = 6318.0,
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
        .health = 10692.0,
        .damage = 12636.0,
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
        .health = 30.0,
        .damage = 36.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.225f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 90.0,
        .damage = 108.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.26f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 270.0,
        .damage = 324.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.295f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 540.0,
        .damage = 648.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.33f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 1080.0,
        .damage = 1296.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.365f }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 2160.0,
        .damage = 2592.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.4f }
    },
    {
        .name = "Sand",
        .description = "It's coarse, rough, and gets everywhere",
        .health = 30.0,
        .damage = 12.0,
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
        .health = 90.0,
        .damage = 36.0,
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
        .health = 270.0,
        .damage = 108.0,
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
        .health = 540.0,
        .damage = 216.0,
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
        .health = 1080.0,
        .damage = 432.0,
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
        .health = 2160.0,
        .damage = 864.0,
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
        .health = 60.0,
        .damage = 51.0,
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
        .health = 180.0,
        .damage = 153.0,
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
        .health = 360.0,
        .damage = 306.0,
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
        .health = 720.0,
        .damage = 612.0,
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
        .health = 1440.0,
        .damage = 1224.0,
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
        .health = 96.0,
        .damage = 48.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 288.0,
        .damage = 144.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 864.0,
        .damage = 432.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 2592.0,
        .damage = 1296.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 5184.0,
        .damage = 2592.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 10368.0,
        .damage = 5184.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Sand Spike",
        .description = "A brittle desert spike with a hardened tip that pierces a small portion of enemy armor.",
        .health = 20736.0,
        .damage = 10368.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor_penetration = 0.12f }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 390.0,
        .damage = 30.0,
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
        .damage = 90.0,
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
        .health = 2340.0,
        .damage = 180.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .damage_reflection = 0.36f,
            .burn_damage = { .damage = 2430.0, .time = 1.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 4680.0,
        .damage = 360.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .damage_reflection = 0.395f,
            .burn_damage = { .damage = 4860.0, .time = 1.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 9360.0,
        .damage = 720.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .damage_reflection = 0.43f,
            .burn_damage = { .damage = 9720.0, .time = 1.0 }
        }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 156.0,
        .damage = 24.0,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 4.45 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 468.0,
        .damage = 72.0,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 5.575 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 1404.0,
        .damage = 216.0,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 6.7 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 4212.0,
        .damage = 648.0,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 7.825 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 8424.0,
        .damage = 1296.0,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 8.95 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 16848.0,
        .damage = 2592.0,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 10.075 }
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 33696.0,
        .damage = 5184.0,
        .radius = 12.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 11.2 }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 180.0,
        .damage = 270.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.3f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 360.0,
        .damage = 540.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.32f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 720.0,
        .damage = 1080.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.34f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 1440.0,
        .damage = 2160.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.36f }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 135.0,
        .damage = 33.0,
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
        .damage = 99.0,
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
        .damage = 297.0,
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
        .damage = 891.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 7290.0,
        .damage = 1782.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 14580.0,
        .damage = 3564.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Snowball",
        .description = "Two fast-returning snowballs that briefly chill enemies, offering light control without sacrificing speed.",
        .health = 29160.0,
        .damage = 7128.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 0.40 }
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 360.0,
        .damage = 9.0,
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
        .health = 1080.0,
        .damage = 27.0,
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
        .health = 2160.0,
        .damage = 54.0,
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
        .health = 4320.0,
        .damage = 108.0,
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
        .health = 8640.0,
        .damage = 216.0,
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
        .health = 21.0,
        .damage = 51.0,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 63.0,
        .damage = 153.0,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 189.0,
        .damage = 459.0,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 378.0,
        .damage = 918.0,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 756.0,
        .damage = 1836.0,
        .radius = 13.0,
        .reload = 3.75,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Spider Fang",
        .description = "A curved fang that bites hard",
        .health = 1512.0,
        .damage = 3672.0,
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
        .damage = 135.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 18.0, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 972.0,
        .damage = 405.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 54.0, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 1944.0,
        .damage = 810.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 108.0, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 3888.0,
        .damage = 1620.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 216.0, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 7776.0,
        .damage = 3240.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 432.0, .time = 3.0 } }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 120.0,
        .damage = 45.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.9797f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 360.0,
        .damage = 135.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.0563f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 1080.0,
        .damage = 405.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.1328f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 2160.0,
        .damage = 810.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.2094f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 4320.0,
        .damage = 1620.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.2859f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 8640.0,
        .damage = 3240.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 2.3625f }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 450.0,
        .damage = 288.0,
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
        .damage = 864.0,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 486.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 4050.0,
        .damage = 2592.0,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 1458.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 8100.0,
        .damage = 5184.0,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 2916.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 16200.0,
        .damage = 10368.0,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 5832.0 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 32400.0,
        .damage = 20736.0,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 11664.0 }
    },
    {
        .name = "Stick",
        .description = "Periodically summons sandstorms to fight for you",
        .health = 30.0,
        .damage = 3.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { 
            .secondary_reload = 2.625,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
        }
    },
    {
        .name = "Stick",
        .description = "Periodically summons sandstorms to fight for you",
        .health = 60.0,
        .damage = 6.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .secondary_reload = 2.35,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
        }
    },
    {
        .name = "Stick",
        .description = "Periodically summons sandstorms to fight for you",
        .health = 120.0,
        .damage = 12.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .secondary_reload = 2.075,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
        }
    },
    {
        .name = "Stick",
        .description = "Periodically summons sandstorms to fight for you",
        .health = 240.0,
        .damage = 24.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .secondary_reload = 1.8,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 72.0,
        .damage = 10.5,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .armor = 11.125
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 216.0,
        .damage = 31.5,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .armor = 13.9375
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 648.0,
        .damage = 94.5,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .armor = 16.75
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 1944.0,
        .damage = 283.5,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .armor = 19.5625
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 3888.0,
        .damage = 567.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .armor = 22.375
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 7776.0,
        .damage = 1134.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .armor = 25.1875
        }
    },
    {
        .name = "Stone Heart",
        .description = "A heavy stone heart that hardens your flower",
        .health = 15552.0,
        .damage = 2268.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .armor = 28.0
        }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 570.0,
        .damage = 15.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 405.0, .poison_armor = 30.15 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 1710.0,
        .damage = 45.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 1215.0, .poison_armor = 35.2125 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 3420.0,
        .damage = 90.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 2430.0, .poison_armor = 40.275 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 6840.0,
        .damage = 180.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 4860.0, .poison_armor = 45.3375 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 13680.0,
        .damage = 360.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 9720.0, .poison_armor = 50.4 }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 20.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_range = 96.25, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 40.0,
        .damage = 20.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_range = 108.125, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 80.0,
        .damage = 40.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_range = 120.0, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 270.0,
        .damage = 72.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.29f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 810.0,
        .damage = 216.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.325f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 1620.0,
        .damage = 432.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.36f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 3240.0,
        .damage = 864.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.395f }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 6480.0,
        .damage = 1728.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.43f }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 45.0,
        .damage = 33.0,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 135.0,
        .damage = 99.0,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 405.0,
        .damage = 297.0,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 810.0,
        .damage = 594.0,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 1620.0,
        .damage = 1188.0,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Ring",
        .description = "A thorny green ring of spikes",
        .health = 3240.0,
        .damage = 2376.0,
        .radius = 15.0,
        .reload = 3.5,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 11 }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 30.0,
        .damage = 34.5,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 1.7812,
            .poison_damage = { .damage = 12.0, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 90.0,
        .damage = 103.5,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 2.025,
            .poison_damage = { .damage = 36.0, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 270.0,
        .damage = 310.5,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 2.2687,
            .poison_damage = { .damage = 108.0, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 540.0,
        .damage = 621.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 2.5125,
            .poison_damage = { .damage = 216.0, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 1080.0,
        .damage = 1242.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 2.7563,
            .poison_damage = { .damage = 432.0, .time = 2.0 }
        }
    },
    {
        .name = "Thorn Trap",
        .description = "Thorny ring that poisons and slows on contact",
        .health = 2160.0,
        .damage = 2484.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 3.0,
            .poison_damage = { .damage = 864.0, .time = 2.0 }
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
        .health = 495.0,
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
        .health = 4455.0,
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
        .health = 8910.0,
        .damage = 1944.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 1.8425, .snare_seconds = 1.34 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 17820.0,
        .damage = 3888.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 2.0213, .snare_seconds = 1.47 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 35640.0,
        .damage = 7776.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 2.2, .snare_seconds = 1.6 }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 84.0,
        .damage = 36.0,
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
        .health = 252.0,
        .damage = 108.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 1.8125,
            .extra_range = 162.0,
            .radial_pulse_damage = 126.0036,
            .radial_pulse_radius = 810.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 756.0,
        .damage = 324.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.6406,
            .extra_range = 486.0,
            .radial_pulse_damage = 378.0108,
            .radial_pulse_radius = 2430.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 1512.0,
        .damage = 648.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.4688,
            .extra_range = 972.0,
            .radial_pulse_damage = 756.0216,
            .radial_pulse_radius = 4860.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 3024.0,
        .damage = 1296.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.2969,
            .extra_range = 1944.0,
            .radial_pulse_damage = 1512.0432,
            .radial_pulse_radius = 9720.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 6048.0,
        .damage = 2592.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.125,
            .extra_range = 3888.0,
            .radial_pulse_damage = 3024.0864,
            .radial_pulse_radius = 19440.0
        }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 54.0,
        .damage = 18.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 11.15 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 162.0,
        .damage = 54.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 13.4 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 486.0,
        .damage = 162.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 15.65 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 972.0,
        .damage = 324.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 17.9 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 1944.0,
        .damage = 648.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 20.15 }
    },
    {
        .name = "Toxic Heart",
        .description = "A hardened heart that armor-plates your petals",
        .health = 3888.0,
        .damage = 1296.0,
        .radius = 15.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 22.4 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 780.0,
        .damage = 24.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 1296.0, .extra_health = 1458.0, .taunt_radius = 260.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 2340.0,
        .damage = 72.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 3888.0, .extra_health = 4374.0, .taunt_radius = 295.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 4680.0,
        .damage = 144.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 7776.0, .extra_health = 8748.0, .taunt_radius = 330.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 9360.0,
        .damage = 288.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 15552.0, .extra_health = 17496.0, .taunt_radius = 365.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 18720.0,
        .damage = 576.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 31104.0, .extra_health = 34992.0, .taunt_radius = 400.0 }
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
        .health = 2430.0,
        .damage = 3888.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 4860.0,
        .damage = 7776.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 9720.0,
        .damage = 15552.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 285.0,
        .damage = 36.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mass = 88.5, .armor = 4.1812 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 855.0,
        .damage = 108.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mass = 106.0, .armor = 5.025 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 2565.0,
        .damage = 324.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mass = 123.5, .armor = 5.8687 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 5130.0,
        .damage = 648.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mass = 141.0, .armor = 6.7125 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 10260.0,
        .damage = 1296.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mass = 158.5, .armor = 7.5562 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 20520.0,
        .damage = 2592.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .mass = 176.0, .armor = 8.4 }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 270.0,
        .damage = 42.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 6.7, .poison_damage = { .damage = 27.0, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 810.0,
        .damage = 126.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 7.825, .poison_damage = { .damage = 81.0, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 1620.0,
        .damage = 252.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 8.95, .poison_damage = { .damage = 162.0, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 3240.0,
        .damage = 504.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 10.075, .poison_damage = { .damage = 324.0, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 6480.0,
        .damage = 1008.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 11.2, .poison_damage = { .damage = 648.0, .time = 2.5 } }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 25.5,
        .damage = 45.0,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 76.5,
        .damage = 135.0,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 229.5,
        .damage = 405.0,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 459.0,
        .damage = 810.0,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 918.0,
        .damage = 1620.0,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Vine Whip",
        .description = "A long vine that lashes farther out",
        .health = 1836.0,
        .damage = 3240.0,
        .radius = 16.0,
        .reload = 3.25,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 12, .extra_range = 40.f }
    },
    {
        .name = "Volcano Worm",
        .description = "Hatches a magma centipede to fight for you.",
        .health = 240.0,
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
        .health = 720.0,
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
        .health = 1440.0,
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
        .health = 2880.0,
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
        .health = 5760.0,
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
        .health = 45.0,
        .damage = 30.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 216.0, .splash_damage = 216.0, .splash_radius = 1485.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 135.0,
        .damage = 90.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 648.0, .splash_damage = 648.0, .splash_radius = 4455.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 405.0,
        .damage = 270.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 1944.0, .splash_damage = 1944.0, .splash_radius = 13365.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 810.0,
        .damage = 540.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 3888.0, .splash_damage = 3888.0, .splash_radius = 26730.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 1620.0,
        .damage = 1080.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 7776.0, .splash_damage = 7776.0, .splash_radius = 53460.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 3240.0,
        .damage = 2160.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 15552.0, .splash_damage = 15552.0, .splash_radius = 106920.0, .defend_only = 1 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 105.0,
        .damage = 66.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {  .move_speed_factor = 1.2488f, .slow_inflict_seconds = 2.7 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 315.0,
        .damage = 198.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {  .move_speed_factor = 1.2834f, .slow_inflict_seconds = 3.025 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 630.0,
        .damage = 396.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {  .move_speed_factor = 1.3181f, .slow_inflict_seconds = 3.35 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 1260.0,
        .damage = 792.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {  .move_speed_factor = 1.3528f, .slow_inflict_seconds = 3.675 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 2520.0,
        .damage = 1584.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {  .move_speed_factor = 1.3875f, .slow_inflict_seconds = 4.0 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 360.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { 
            .secondary_reload = 0.3281,
            .aura_heal = 20.9952,
            .aura_radius = 465.0048,
            .aura_duration = 15.65,
            .defend_only = 1, .constant_heal = 9.0 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 720.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { 
            .secondary_reload = 0.2937,
            .aura_heal = 41.9904,
            .aura_radius = 930.0096,
            .aura_duration = 17.9,
            .defend_only = 1, .constant_heal = 18.0 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 1440.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { 
            .secondary_reload = 0.2594,
            .aura_heal = 83.9808,
            .aura_radius = 1860.0192,
            .aura_duration = 20.15,
            .defend_only = 1, .constant_heal = 36.0 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 2880.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .secondary_reload = 0.225,
            .aura_heal = 167.9616,
            .aura_radius = 3720.0384,
            .aura_duration = 22.4,
            .defend_only = 1, .constant_heal = 72.0 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 60.0,
        .damage = 60.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.525 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 180.0,
        .damage = 180.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.6188 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 360.0,
        .damage = 360.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.7125 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 720.0,
        .damage = 720.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.8063 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 1440.0,
        .damage = 1440.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.9 }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 30.0,
        .damage = 15.0,
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
        .health = 90.0,
        .damage = 45.0,
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
        .health = 270.0,
        .damage = 135.0,
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
        .health = 810.0,
        .damage = 405.0,
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
        .health = 1620.0,
        .damage = 810.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .constant_heal = 243.0,
            .icon_angle = -1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 3240.0,
        .damage = 1620.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .constant_heal = 486.0,
            .icon_angle = -1
        }
    },
    {
        .name = "Yucca",
        .description = "Heals the flower, but only while in the defensive position",
        .health = 6480.0,
        .damage = 3240.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .constant_heal = 972.0,
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
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 17496.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Abyss Pearl",
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
        .health = 34992.0,
        .damage = 23328.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 0.9877,
        .damage = 0.679,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 3.5,
            .slow_inflict_seconds = 0.595,
            .extra_range = 0.5185,
            .radial_pulse_damage = 0.3457,
            .radial_pulse_radius = 1.7284
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 2.963,
        .damage = 2.037,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 3.2594,
            .slow_inflict_seconds = 0.7331,
            .extra_range = 1.5555,
            .radial_pulse_damage = 1.0371,
            .radial_pulse_radius = 5.1852
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 8.8889,
        .damage = 6.1111,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 3.0187,
            .slow_inflict_seconds = 0.8712,
            .extra_range = 4.6665,
            .radial_pulse_damage = 3.1113,
            .radial_pulse_radius = 15.5556
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 26.6667,
        .damage = 18.3333,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.7781,
            .slow_inflict_seconds = 1.0094,
            .extra_range = 13.9995,
            .radial_pulse_damage = 9.3339,
            .radial_pulse_radius = 46.6668
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
        .health = 13608.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 27216.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ancient Heart",
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
        .health = 54432.0,
        .damage = 34992.0,
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
        .damage = 5.0,
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
        .damage = 15.0,
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
        .health = 5.0,
        .damage = 1.1111,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 0.5556, .on_hit_heal = 1.6625 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 15.0,
        .damage = 3.3333,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 1.6667, .on_hit_heal = 2.225 }
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
        .health = 3.7037,
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
        .health = 11.1111,
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
        .health = 33.3333,
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ant Roe",
        .description = "Ant egg cluster. Forges Soldier Egg.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ant Shell",
        .description = "Chitin shell from worker ants. Forges Ant Fang.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Antenna Scrap",
        .description = "Antenna scrap from ants. Forges Ant Fang.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kCommon,
        .attributes = { .vision_factor = 0.85f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 0.1852,
        .damage = 0.1111,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kUnusual,
        .attributes = { .vision_factor = 0.7937f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 0.5556,
        .damage = 0.3333,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kRare,
        .attributes = { .vision_factor = 0.7375f, .equipment = EquipmentFlags::kAntennae }
    },
    {
        .name = "Antennae",
        .description = "Allows your flower to sense foes from farther away",
        .health = 1.6667,
        .damage = 1.0,
        .radius = 12.5,
        .reload = 0.0,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Bee Barb",
        .description = "Tiny barb from bees and hornets. Forges Bee Sting.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 17496.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Blizzard Eye",
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
        .health = 34992.0,
        .damage = 23328.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 0.4938,
        .damage = 0.0988,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 8.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 1.4815,
        .damage = 0.2963,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 13.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 4.4444,
        .damage = 0.8889,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 18.0, .icon_angle = 1.0 }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 13.3333,
        .damage = 2.6667,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 23.0, .icon_angle = 1.0 }
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
        .reload = 1.75,
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
            .extra_health = 20, .damage_reflection = 0.1f }
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Cactus Sap",
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Capital Badge",
        .description = "Badge from sentinels. Forges Holy Orb.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Carapace",
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
        .health = 23328.0,
        .damage = 15552.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 0.036,
        .damage = 0.0257,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .poison_damage = { .damage = 0.0864, .time = 2.5 },
            .slow_inflict_seconds = 0.525
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 0.108,
        .damage = 0.0772,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .poison_damage = { .damage = 0.2592, .time = 2.5 },
            .slow_inflict_seconds = 0.6469
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 0.3241,
        .damage = 0.2315,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .poison_damage = { .damage = 0.7776, .time = 2.5 },
            .slow_inflict_seconds = 0.7687
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 0.9722,
        .damage = 0.6944,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .poison_damage = { .damage = 2.3328, .time = 2.5 },
            .slow_inflict_seconds = 0.8906
        }
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
        .health = 1.1111,
        .damage = 0.2469,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 0.5,
            .slow_inflict_seconds = 1.4,
            .aura_radius = 1.4815,
            .aura_duration = 3.3,
            .aura_slow_seconds = 0.84,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 3.3333,
        .damage = 0.7407,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 0.4656,
            .slow_inflict_seconds = 1.725,
            .aura_radius = 4.4445,
            .aura_duration = 4.9875,
            .aura_slow_seconds = 1.035,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 10.0,
        .damage = 2.2222,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.4313,
            .slow_inflict_seconds = 2.05,
            .aura_radius = 13.3335,
            .aura_duration = 6.675,
            .aura_slow_seconds = 1.23,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 30.0,
        .damage = 6.6667,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.3969,
            .slow_inflict_seconds = 2.375,
            .aura_radius = 40.0005,
            .aura_duration = 8.3625,
            .aura_slow_seconds = 1.425,
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Cold Iron",
        .description = "Frost iron from ice beasts. Forges Ice Spike.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .health = 5.5556,
        .damage = 0.7778,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .move_speed_factor = 1.1138f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 16.6667,
        .damage = 2.3333,
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
        .health = 20.0,
        .damage = 8.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 0.9313,
            .constant_heal = 0.1482,
            .burst_heal = 9.975,
            .extra_rotation_speed = 0.36
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 60.0,
        .damage = 24.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.8625,
            .constant_heal = 0.4446,
            .burst_heal = 13.35,
            .extra_rotation_speed = 0.48
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 180.0,
        .damage = 72.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.7937,
            .constant_heal = 1.3338,
            .burst_heal = 16.725,
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Coral",
        .description = "Branching reef coral. Forges Starfish.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .health = 4.4444,
        .damage = 1.3333,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 20.0, .poison_armor = 4.9875 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 13.3333,
        .damage = 4.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 20.0, .poison_armor = 6.675 }
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
        .health = 50.0,
        .damage = 5.0,
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
        .damage = 15.0,
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
        .health = 4.4444,
        .damage = 2.5556,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 0.6666, .time = 5.0004 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 13.3333,
        .damage = 7.6667,
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
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 17496.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Corrupt Spore",
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
        .health = 34992.0,
        .damage = 23328.0,
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
        .attributes = { .damage_reflection = 0.2f, .extra_health = 12.0 }
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
        .damage = 8.0,
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
        .damage = 24.0,
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
        .damage = 72.0,
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
        .damage = 216.0,
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
        .damage = 648.0,
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
        .health = 1944.0,
        .damage = 1296.0,
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
        .health = 3888.0,
        .damage = 2592.0,
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
        .health = 1.6667,
        .damage = 2.7778,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_body_damage = 17.375, .equipment = EquipmentFlags::kCutter }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 5.0,
        .damage = 8.3333,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_body_damage = 22.75, .equipment = EquipmentFlags::kCutter }
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
        .health = 9.4444,
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
        .health = 28.3333,
        .damage = 10.0,
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
        .health = 17.7778,
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
        .health = 53.3333,
        .damage = 1.6667,
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
        .health = 3.3333,
        .damage = 2.6667,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Fire Crystal",
        .description = "Fire crystal from ember wasps. Forges Fireball.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .health = 6.6667,
        .damage = 3.7778,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_rotation_speed = 0.225, .extra_range = 0.8889 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 20.0,
        .damage = 11.3333,
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
        .health = 6.1111,
        .damage = 5.3333,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_damage_factor = 1.14f, .armor_penetration = 0.2875f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 18.3333,
        .damage = 16.0,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Formic Acid",
        .description = "Formic acid from soldiers. Forges Acid Flask.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .attributes = {  .armor = 8.0, .defend_only = 1, .slow_inflict_seconds = 1.4 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 2.7778,
        .damage = 2.6667,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {  .armor = 24.0, .defend_only = 1, .slow_inflict_seconds = 1.725 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 8.3333,
        .damage = 8.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {  .armor = 72.0, .defend_only = 1, .slow_inflict_seconds = 2.05 }
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
        .attributes = { .extra_health = 32.0, .poison_armor = 4.4 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 16.6667,
        .damage = 0.5556,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 96.0, .poison_armor = 6.65 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 50.0,
        .damage = 1.6667,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 288.0, .poison_armor = 8.9 }
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
        .attributes = { .extra_health = 12.0, .slow_inflict_seconds = 1.4 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 5.5556,
        .damage = 4.6667,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 36.0, .slow_inflict_seconds = 1.725 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 16.6667,
        .damage = 14.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_health = 108.0, .slow_inflict_seconds = 2.05 }
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
        .health = 5.0,
        .damage = 2.8889,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost-touched starfish that briefly chill enemies on contact, keeping their prey within reach.",
        .health = 15.0,
        .damage = 8.6667,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Gear Tooth",
        .description = "Precision gear from rust mites. Forges Mech Gear.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .damage = 5.3333,
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
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 1.6667,
        .damage = 1.1111,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_reload_factor = 0.97f, .constant_heal = 1.0, .icon_angle = -1.0 }
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 5.0,
        .damage = 3.3333,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_reload_factor = 0.9512f, .constant_heal = 1.625, .icon_angle = -1.0 }
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
        .health = 35.0,
        .damage = 1.0,
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
        .damage = 3.0,
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
        .health = 2.716,
        .damage = 0.0617,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .constant_heal = 0.0617, .extra_health = 0.4321, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 8.1481,
        .damage = 0.1852,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 0.1852, .extra_health = 1.2963, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 24.4444,
        .damage = 0.5556,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 0.5556, .extra_health = 3.8889, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 73.3333,
        .damage = 1.6667,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 1.6667, .extra_health = 11.6667, .defend_only = 1 }
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
        .health = 40.0,
        .damage = 12.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
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
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 360.0,
        .damage = 108.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
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
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 3240.0,
        .damage = 972.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 6480.0,
        .damage = 1944.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
    },
    {
        .name = "Heartwood",
        .description = "The living heart of an ancient treant. Throw to root it — it radiates restorative energy to nearby allies.",
        .health = 12960.0,
        .damage = 3888.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 0.45, .aura_heal = 14.0, .aura_radius = 145.0, .aura_duration = 16.0, .defend_only = 1 }
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
        .health = 13608.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 27216.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Heartwood Chip",
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
        .health = 54432.0,
        .damage = 34992.0,
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
        .health = 7.4074,
        .damage = 0.3704,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 22.2222,
        .damage = 1.1111,
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
        .attributes = { .armor = 4.9875, .extra_rotation_speed = 0.225 }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 46.6667,
        .damage = 9.0,
        .radius = 13.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 6.675, .extra_rotation_speed = 0.3 }
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
        .health = 13608.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 27216.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Hive Heart",
        .description = "Queen Ant sovereign core. Keys Ant King.",
        .health = 54432.0,
        .damage = 34992.0,
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
        .damage = 2.7778,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .secondary_reload = 0.9313, .burst_heal = 108.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 23.3333,
        .damage = 8.3333,
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Holy Shard",
        .description = "Holy shard from royal bees. Forges Holy Orb.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .attributes = {  .secondary_reload = 1.0, .burst_heal = 16.0, .defend_only = 1, .constant_heal = 0.1111 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 4.4444,
        .damage = 0.5556,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {  .secondary_reload = 0.9313, .burst_heal = 48.0, .defend_only = 1, .constant_heal = 0.3333 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 13.3333,
        .damage = 1.6667,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {  .secondary_reload = 0.8625, .burst_heal = 144.0, .defend_only = 1, .constant_heal = 1.0 }
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
        .damage = 0.4444,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .constant_heal = 0.2222,
            .poison_damage = { .damage = 0.3333, .time = 2.9997 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 10.0,
        .damage = 1.3333,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .constant_heal = 0.6667,
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ice Crystal",
        .description = "Snow scrap from ice bugs. Forges Ice Spike.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Ice Scale",
        .description = "Scale of polar beasts. Forges Ice Shield.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .health = 36.6667,
        .damage = 2.0,
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
        .health = 5.5556,
        .damage = 2.0,
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
        .health = 16.6667,
        .damage = 6.0,
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
        .health = 2.2222,
        .damage = 0.8025,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .extra_health = 25.0,
            .burn_damage = { .damage = 0.0988, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 6.6667,
        .damage = 2.4074,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_health = 75.0,
            .burn_damage = { .damage = 0.2963, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 20.0,
        .damage = 7.2222,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_health = 225.0,
            .burn_damage = { .damage = 0.8889, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 60.0,
        .damage = 21.6667,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_health = 675.0,
            .burn_damage = { .damage = 2.6667, .time = 4.0 }
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
        .health = 13.3333,
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
        .health = 10.0,
        .damage = 6.6667,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .extra_damage_factor = 1.15f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 30.0,
        .damage = 20.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_damage_factor = 1.1787f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 90.0,
        .damage = 60.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_damage_factor = 1.2075f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 270.0,
        .damage = 180.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .extra_damage_factor = 1.2362f
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Lava Rock",
        .description = "Warm lava rock from magma crabs. Forges Magma Core.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Leaf Scrap",
        .description = "Garden scrap from beetles and ladybugs. Forges Leaf Blade.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .health = 13608.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 27216.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Leyline Core",
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
        .health = 54432.0,
        .damage = 34992.0,
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
        .health = 5.5556,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .constant_heal = 0.4444, .extra_health = 45.0, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 16.6667,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 1.3333, .extra_health = 135.0, .defend_only = 1 }
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
        .attributes = { .poison_armor = 6.0, .constant_heal = 1.0, .icon_angle = 0.1 }
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
        .attributes = { .poison_armor = 9.75, .constant_heal = 1.875, .icon_angle = 0.1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 2.2222,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_armor = 13.5, .constant_heal = 2.75, .icon_angle = 0.1 }
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
        .attributes = {  .extra_health = 10.0, .splash_damage = 12.0, .splash_radius = 70.0, .burn_damage = { .damage = 0.6667, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 75.0,
        .damage = 28.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {  .extra_health = 30.0, .splash_damage = 36.0, .splash_radius = 210.0, .burn_damage = { .damage = 2.0, .time = 3.0 } }
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
        .health = 80.0,
        .damage = 20.0,
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
        .damage = 5.3333,
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
        .health = 4.4444,
        .damage = 4.4444,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
    },
    {
        .name = "Molten Sand Blade",
        .description = "An unstable blade forged from molten sand. Its heavy strikes leave a lingering burn.",
        .health = 13.3333,
        .damage = 13.3333,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .burn_damage = { .damage = 6.0, .time = 3.0 } }
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
        .attributes = { .aura_heal = 4.0, .aura_radius = 140.0, .aura_duration = 12.0, .secondary_reload = 0.7, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Monstera",
        .description = "A fenestrated jungle leaf. Throw it on the ground — it roots and heals allies in range.",
        .health = 1.1111,
        .damage = 0.1852,
        .radius = 15.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .aura_heal = 6.25, .aura_radius = 175.0, .aura_duration = 14.0, .secondary_reload = 0.6562, .defend_only = 1, .icon_angle = -0.35f }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 0.1029,
        .damage = 0.0617,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .mass = 50.0 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 0.3086,
        .damage = 0.1852,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mass = 71.25 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 0.9259,
        .damage = 0.5556,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 92.5 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 2.7778,
        .damage = 1.6667,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .mass = 113.75 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 8.3333,
        .damage = 5.0,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .mass = 135.0 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 25.0,
        .damage = 15.0,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .mass = 156.25 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 50.0,
        .damage = 30.0,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .mass = 177.5 }
    },
    {
        .name = "Moon",
        .description = "Where did this come from?",
        .health = 100.0,
        .damage = 60.0,
        .radius = 50.0,
        .reload = 10.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .mass = 198.75 }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 0.0494,
        .damage = 0.0206,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kCommon,
        .attributes = { .vision_factor = 0.55f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 0.1481,
        .damage = 0.0617,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kUnusual,
        .attributes = { .vision_factor = 0.5038f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 0.4444,
        .damage = 0.1852,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kRare,
        .attributes = { .vision_factor = 0.4575f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 1.3333,
        .damage = 0.5556,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kEpic,
        .attributes = { .vision_factor = 0.4113f, .equipment = EquipmentFlags::kObserver }
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 4.0,
        .damage = 1.6667,
        .radius = 12.5,
        .reload = 0.0,
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
        .damage = 34.0,
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
        .health = 80.0,
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Obsidian Shard",
        .description = "Obsidian from golems. Forges Obsidian Blade.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .damage = 3.1111,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 12.0, .extra_range = 3.3333 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 23.3333,
        .damage = 9.3333,
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
        .health = 13608.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 27216.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Permafrost Heart",
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
        .health = 54432.0,
        .damage = 34992.0,
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
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 17496.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Phoenix Ember",
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
        .health = 34992.0,
        .damage = 23328.0,
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
                .damage = 1.6667,
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
        .health = 6.1111,
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
        .health = 18.3333,
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Pollen Dust",
        .description = "Sticky golden pollen from bees. Forges Healing Bloom.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .attributes = { 
            .secondary_reload = 0.5,
            .aura_heal = 0.1111,
            .aura_radius = 3.5185,
            .aura_duration = 2.2,
            .defend_only = 1, .slow_inflict_seconds = 2.1 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 4.4444,
        .damage = 0.8889,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .secondary_reload = 0.4656,
            .aura_heal = 0.3333,
            .aura_radius = 10.5555,
            .aura_duration = 3.325,
            .defend_only = 1, .slow_inflict_seconds = 2.5875 }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 13.3333,
        .damage = 2.6667,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { 
            .secondary_reload = 0.4313,
            .aura_heal = 0.9999,
            .aura_radius = 31.6665,
            .aura_duration = 4.45,
            .defend_only = 1, .slow_inflict_seconds = 3.075 }
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
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 17496.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Queen Pheromone",
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
        .health = 34992.0,
        .damage = 23328.0,
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
        .health = 25.0,
        .damage = 5.0,
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
        .health = 75.0,
        .damage = 15.0,
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
        .health = 225.0,
        .damage = 45.0,
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
        .health = 7.3333,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Refined Steel",
        .description = "Steel from capital guards. Forges Steel Sword.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Resin",
        .description = "Amber resin from treants. Forges Rotwood.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .health = 5.0,
        .damage = 8.0,
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
        .health = 15.0,
        .damage = 24.0,
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
            .burst_heal = 14,
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
        .health = 40.0,
        .damage = 2.6667,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 2.0, .time = 2.0 } }
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
        .health = 20.0,
        .damage = 3.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 8.3125, .extra_health = 20.7812 }
    },
    {
        .name = "Royal Carapace",
        .description = "Gold-lined royal plate with heavy protection.",
        .health = 60.0,
        .damage = 9.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 11.125, .extra_health = 27.8125 }
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
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 17496.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Royal Jelly",
        .description = "Side-core of the Flower Guardian. Keys Heart of Nature.",
        .health = 34992.0,
        .damage = 23328.0,
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
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 17496.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Royal Seal",
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
        .health = 34992.0,
        .damage = 23328.0,
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
            .extra_range = 5.0f,
            .icon_angle = 0.5
        }
    },
    {
        .name = "Royal Spear",
        .description = "A long royal spear with extended reach",
        .health = 3.0,
        .damage = 6.6667,
        .radius = 17.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_range = 15.0f,
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
        .health = 6.1111,
        .damage = 4.0,
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
        .health = 18.3333,
        .damage = 12.0,
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
        .damage = 4.0,
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sand Grain",
        .description = "Desert scrap from mites and dunes. Forges Sand Spike.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .health = 2.2222,
        .damage = 1.8889,
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
        .health = 6.6667,
        .damage = 5.6667,
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
        .health = 8748.0,
        .damage = 5832.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 17496.0,
        .damage = 11664.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sandstorm Eye",
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
        .health = 34992.0,
        .damage = 23328.0,
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
        .health = 13608.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 27216.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sandworm Core",
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
        .health = 54432.0,
        .damage = 34992.0,
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
        .damage = 1.1111,
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
        .health = 43.3333,
        .damage = 3.3333,
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sea Salt",
        .description = "Sea salt from crabs and shallows. Forges Water Bubble.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Seashell",
        .description = "Colorful shell from reef dwellers. Forges Starfish.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .health = 0.7407,
        .damage = 1.1111,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .extra_damage_factor = 1.28f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 2.2222,
        .damage = 3.3333,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_damage_factor = 1.28f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 6.6667,
        .damage = 10.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_damage_factor = 1.28f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 20.0,
        .damage = 30.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_damage_factor = 1.28f }
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Snow Fluff",
        .description = "Soft fluff from snow hares. Forges Snowball.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .health = 13.3333,
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
        .health = 40.0,
        .damage = 1.0,
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
        .health = 13608.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 27216.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sovereign Core",
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
        .health = 54432.0,
        .damage = 34992.0,
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
        .damage = 5.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 0.6667, .time = 3.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 36.0,
        .damage = 15.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 2.0, .time = 3.0 } }
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Spore Dust",
        .description = "Spore dust from fungi. Forges Poison Spore.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 30.0,
        .damage = 10.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 90.0,
        .damage = 30.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 270.0,
        .damage = 90.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 810.0,
        .damage = 270.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 2430.0,
        .damage = 810.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 4860.0,
        .damage = 1620.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
        }
    },
    {
        .name = "Square",
        .description = "This shape... it looks familiar...",
        .health = 9720.0,
        .damage = 3240.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .icon_angle = M_PI / 4 + 1
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
        .health = 13.3333,
        .damage = 5.0,
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
        .health = 50.0,
        .damage = 32.0,
        .radius = 12.0,
        .reload = 1.9,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_health = 18.0 }
    },
    {
        .name = "Stick",
        .description = "Periodically summons sandstorms to fight for you",
        .health = 0.1235,
        .damage = 0.0123,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { 
            .secondary_reload = 4.0,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
        }
    },
    {
        .name = "Stick",
        .description = "Periodically summons sandstorms to fight for you",
        .health = 0.3704,
        .damage = 0.037,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .secondary_reload = 3.725,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
        }
    },
    {
        .name = "Stick",
        .description = "Periodically summons sandstorms to fight for you",
        .health = 1.1111,
        .damage = 0.1111,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { 
            .secondary_reload = 3.45,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
        }
    },
    {
        .name = "Stick",
        .description = "Periodically summons sandstorms to fight for you",
        .health = 3.3333,
        .damage = 0.3333,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .secondary_reload = 3.175,
            .defend_only = 1,
            .icon_angle = 1,
            .spawns = MobID::kSandstorm,
            .spawn_count = 2
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sulfur",
        .description = "Sulfur from ash and cinders. Forges Fireball.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .health = 21.1111,
        .damage = 0.5556,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 15.0, .poison_armor = 14.9625 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 63.3333,
        .damage = 1.6667,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 45.0, .poison_armor = 20.025 }
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
        .health = 0.1235,
        .damage = 0.0617,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_range = 36.875, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 0.3704,
        .damage = 0.1852,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_range = 48.75, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 1.1111,
        .damage = 0.5556,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_range = 60.625, .equipment = EquipmentFlags::kThirdEye }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 3.3333,
        .damage = 1.6667,
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
        .health = 10.0,
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
        .health = 30.0,
        .damage = 8.0,
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
        .health = 5.0,
        .damage = 3.6667,
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
        .damage = 3.8333,
        .radius = 14.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .clump_radius = 10,
            .slow_inflict_seconds = 1.2937,
            .poison_damage = { .damage = 1.3333, .time = 2.0 }
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
        .health = 13608.0,
        .damage = 8748.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 27216.0,
        .damage = 17496.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Tide Core",
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
        .health = 54432.0,
        .damage = 34992.0,
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
        .damage = 4.0,
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
        .attributes = { .armor = 6.65 }
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
        .health = 28.8889,
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
        .health = 86.6667,
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
        .health = 31.6667,
        .damage = 4.0,
        .radius = 13.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .mass = 53.5, .armor = 2.4937 }
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Twig",
        .description = "Dry garden twig shed by beetles. Forges Leaf Blade.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .health = 3888.0,
        .damage = 2430.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 7776.0,
        .damage = 4860.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Venom Barb",
        .description = "Desert barb from scorpions. Forges Sand Spike.",
        .health = 15552.0,
        .damage = 9720.0,
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
        .health = 10.0,
        .damage = 3.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 3.325, .poison_damage = { .damage = 1.0, .time = 2.5 } }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 30.0,
        .damage = 9.0,
        .radius = 12.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 4.45, .poison_damage = { .damage = 3.0, .time = 2.5 } }
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
        .health = 5832.0,
        .damage = 3888.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 11664.0,
        .damage = 7776.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Venom Sac",
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
        .health = 23328.0,
        .damage = 15552.0,
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Vine Strand",
        .description = "Living vine from jungle crawlers. Forges Thorn Vine.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .damage = 5.0,
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
        .health = 8.8889,
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
        .health = 26.6667,
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
        .health = 5.0,
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
        .health = 2430.0,
        .damage = 1458.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 4860.0,
        .damage = 2916.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Water Drop",
        .description = "Ocean scrap from shore creatures. Forges Water Bubble.",
        .health = 9720.0,
        .damage = 5832.0,
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
        .attributes = {  .move_speed_factor = 1.11f, .slow_inflict_seconds = 1.4 }
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
        .attributes = {  .move_speed_factor = 1.1447f, .slow_inflict_seconds = 1.725 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 11.6667,
        .damage = 7.3333,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {  .move_speed_factor = 1.1794f, .slow_inflict_seconds = 2.05 }
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
        .health = 1.4815,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { 
            .secondary_reload = 0.5,
            .aura_heal = 0.0864,
            .aura_radius = 1.9136,
            .aura_duration = 4.4,
            .defend_only = 1, .constant_heal = 0.037 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 4.4444,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { 
            .secondary_reload = 0.4656,
            .aura_heal = 0.2592,
            .aura_radius = 5.7408,
            .aura_duration = 6.65,
            .defend_only = 1, .constant_heal = 0.1111 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 13.3333,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { 
            .secondary_reload = 0.4313,
            .aura_heal = 0.7776,
            .aura_radius = 17.2224,
            .aura_duration = 8.9,
            .defend_only = 1, .constant_heal = 0.3333 }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 40.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { 
            .secondary_reload = 0.3969,
            .aura_heal = 2.3328,
            .aura_radius = 51.6672,
            .aura_duration = 11.15,
            .defend_only = 1, .constant_heal = 1.0 }
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
        .health = 5.5556,
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
        .health = 16.6667,
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
        .health = 50.0,
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
        .health = 150.0,
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
        .health = 300.0,
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
        .health = 600.0,
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
        .health = 2.2222,
        .damage = 2.2222,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .extra_rotation_speed = 0.2437 }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 6.6667,
        .damage = 6.6667,
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
            PetalID::kLight, PetalID::kRose, PetalID::kTwin, PetalID::kBubble,
            PetalID::kMatLeaf
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
            PetalID::kIris, PetalID::kSalt, PetalID::kWing, PetalID::kTriplet,
            PetalID::kMatTwig
        },
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
        // Was 75 — with MOB_RADIUS_MULTIPLIER×rarity scale Legendary filled the FOV.
        .radius = {52.0},
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
            PetalID::kHeavy, PetalID::kLight, PetalID::kRock, PetalID::kMeteor,
            PetalID::kMoon
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
            PetalID::kIris, PetalID::kPincer, PetalID::kTriplet, PetalID::kLotus,
            PetalID::kMatVenomBarb
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
        .description = "Sleeping sovereign of Ant Hell. Drops Hive Heart and Queen Pheromone.",
        .rarity = RarityID::kRare,
        .health = {350.0},
        .damage = 10.0,
        .radius = {25.0},
        .xp = 15,
        // Boss table: signature petals only (mats come from trash ants / shop).
        .drops = {
            PetalID::kColonyHeart, PetalID::kQueensEmblem, PetalID::kAntEgg, PetalID::kTriplet,
            PetalID::kThirdEye, PetalID::kMatHiveHeart, PetalID::kMatQueenPheromone
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
        .drops = { PetalID::kRustNeedle, PetalID::kThornTrap, PetalID::kMatGear },
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
        .drops = { PetalID::kAncientCoin, PetalID::kGuardEgg, PetalID::kMatGear },
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
        .drops = { PetalID::kPhantomEye, PetalID::kKingsOath, PetalID::kMatHolyShard },
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
            PetalID::kLeafBlade, PetalID::kEmeraldShell, PetalID::kMonstera, PetalID::kMatResin
        },
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
        .drops = { PetalID::kEmeraldShell, PetalID::kThornRing },
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
        .rarity = RarityID::kUnusual,
        .health = {32.0}, .damage = 20.0, .radius = {29.3}, .xp = 8,
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
        .drops = {
            PetalID::kPincer, PetalID::kHeavy, PetalID::kRock, PetalID::kMatSeashell
        },
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
            PetalID::kWing, PetalID::kFaster, PetalID::kDandelion, PetalID::kMatSulfur
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
            PetalID::kBeetleEgg, PetalID::kHeavy, PetalID::kSand, PetalID::kMatSand,
            PetalID::kMatCarapace
        },
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
