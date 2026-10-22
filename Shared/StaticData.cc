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
        .description = "Two light petals in one slot",
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
        .description = "Prevents healing on units it hits for a short time",
        .health = 10.0,
        .damage = 10.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .dandelion_inflict_seconds = 10.0f,
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
        .description = "Plant sticky webs that snare foes when defending",
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
        .description = "Orbits outward, then returns",
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
        .description = "Poisons and slows targets for a short duration",
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
        .description = "Three light petals in one slot",
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
        .description = "Periodically summons sandstorms to fight for you",
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
        .description = "Plant sticky webs that snare foes when defending",
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
        .health = 22.0,
        .damage = 5.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .poison_armor = 12,
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
            // Flat add on flattened body curve (~+28% at L100 body≈89).
            .extra_body_damage = 25,
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
    // Golden Leaf (黄金之叶): global petal CD cut; factors multiply across slots.
    // Rare−5% · Epic−8% · Leg−11% · Mythic−14% · Ultra−18% · Super−22% · Unique−26%.
    // World drop: Ultra only from Unique sovereigns (Molten Overlord / Ancient Treant).
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 12.0,
        .damage = 6.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .extra_reload_factor = 0.95f, // -5%
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
        .description = "A sturdy armored shield petal that can intercept missiles",
        .health = 36.0,
        .damage = 5.5,
        .radius = 17.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = { .armor = 3 }
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
        .description = "Inspires faster petal spin while equipped",
        .health = 14.0,
        .damage = 5.5,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .extra_rotation_speed = 0.35f
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
        .description = "Amplifies your petal damage while equipped",
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
        .description = "Thorny ring that poisons and slows on contact",
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
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 13.0,
        .damage = 26.0,
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
    { .name = "Wing", .description = "Orbits outward, then returns", .health = 5, .damage = 5, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUnusual, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "Orbits outward, then returns", .health = 45, .damage = 45, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kEpic, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "Orbits outward, then returns", .health = 135, .damage = 135, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kLegendary, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "Orbits outward, then returns", .health = 405, .damage = 405, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kMythic, .attributes = { .icon_angle = 1 } },
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
    { .name = "Web", .description = "Plant sticky webs that snare foes when defending", .health = 3.33f, .damage = 1.67f, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kUnusual, .attributes = { .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Web", .description = "Plant sticky webs that snare foes when defending", .health = 30, .damage = 15, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kEpic, .attributes = { .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Web", .description = "Plant sticky webs that snare foes when defending", .health = 270, .damage = 135, .radius = 10, .reload = 3.0, .count = 3, .rarity = RarityID::kMythic, .attributes = { .clump_radius = 10, .secondary_reload = 0.5, .defend_only = 1 } },
    // ---- Jungle petals (v1 data-driven) ----
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 7.5, .damage = 9.0, .radius = 12.0, .reload = 1.875, .count = 3,
        .rarity = RarityID::kCommon,
        .attributes = { .clump_radius = 10 }
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
        .health = 40.0, .damage = 5.0, .radius = 17.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .armor = 5 }
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
        .description = "A hardened heart that armor-plates your petals",
        .health = 18.0, .damage = 6.0, .radius = 15.0, .reload = 3.75, .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor = 8 }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 8.0, .damage = 14.0, .radius = 16.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 6.0, .time = 3.0 } }
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
    { .name = "Twin", .description = "Two light petals in one slot", .health = 15, .damage = 24, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kRare, .attributes = {} },
    { .name = "Twin", .description = "Two light petals in one slot", .health = 45, .damage = 72, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kEpic, .attributes = {} },
    { .name = "Twin", .description = "Two light petals in one slot", .health = 135, .damage = 216, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kLegendary, .attributes = {} },
    { .name = "Twin", .description = "Two light petals in one slot", .health = 405, .damage = 648, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kMythic, .attributes = {} },
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
        .description = "Flower Guardian sovereign core. Keys Heart of Nature.",
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
        .description = "Thick sap from desert cactus. Forges Craft Cactus.",
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
        .description = "Hard plate from dune beetles. Forges Craft Cactus.",
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
        .description = "Side-eye of the Sandworm Monarch. Keys Chrono Vortex.",
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
        .description = "Sandworm Monarch sovereign core. Keys Volcano Worm.",
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
        .description = "Branching reef coral. Forges Starfish.",
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
        .description = "Side-pearl of the Abyss Maw. Keys Abyssal Tide.",
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
        .description = "Abyss Maw sovereign core. Keys Abyssal Tide.",
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
        .description = "Scale of polar beasts. Forges Ice Shield.",
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
        .description = "Winter Sovereign core. Keys Ice Tide Ring.",
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
        .description = "Side-eye of the Winter Sovereign. Keys Abyssal Tide.",
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
        .description = "Venom sac from jungle hunters. Forges Poison Spore.",
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
        .description = "Spore dust from fungi. Forges Poison Spore.",
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
        .description = "Ancient Treant sovereign chip. Keys Heart of Nature.",
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
        .description = "Corrupt spore from elite fungi. Feeds border rites.",
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
        .description = "Formic acid from soldiers. Forges Acid Flask.",
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
        .description = "Ant egg cluster. Forges Soldier Egg.",
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
        .description = "Side-scent of the Queen Ant. Keys Ant King.",
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
        .description = "Queen Ant sovereign core. Keys Ant King.",
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
        .description = "Fire crystal from ember wasps. Forges Fireball.",
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
        .description = "Obsidian from golems. Forges Obsidian Blade.",
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
        .description = "Molten Overlord sovereign core. Keys Infernal Core.",
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
        .description = "Side-ember of the Molten Overlord. Keys Infernal Core.",
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
        .description = "Steel from capital guards. Forges Steel Sword.",
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
        .description = "Precision gear from rust mites. Forges Mech Gear.",
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
        .description = "Holy shard from royal bees. Forges Holy Orb.",
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
        .description = "Badge from sentinels. Forges Holy Orb.",
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
        .description = "Fallen King sovereign core. Keys Chrono Vortex.",
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
        .description = "Side-seal of the Fallen King. Keys Skyblade.",
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
        .description = "Brief poison sting. Garden bee-rite weapon.",
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
        .description = "Gentle heal while equipped. Garden bloom rite.",
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
        .description = "Brittle hard spike. Desert traveler's dart.",
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
        .description = "Reflects a little damage. Desert carapace craft.",
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
        .description = "Slows on contact. Sealed with a sandstorm eye.",
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
        .description = "Absorbs and splashes. Ocean local forge.",
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
        .description = "Third hit hits harder. Reef local forge.",
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
        .description = "Orbit + water ring. Ocean tide craft.",
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
        .description = "Slows on hit. Snow traveler's spike.",
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
        .description = "Fast-returning snowballs. Snow local forge.",
        .health = 18.0,
        .damage = 8.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUnusual,
        .attributes = {}
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
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
        .description = "Snares and slows. Jungle local forge.",
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
        .description = "Lingering poison. Jungle rotwood craft.",
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
        .description = "Bonus vs poisoned. Ant Hell local forge.",
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
        .description = "Long poison. Ant Hell acid craft.",
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
        .description = "Hatches a soldier ant. Colony egg rite.",
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
        .description = "Burns on contact. Volcano local forge.",
        .health = 16.0,
        .damage = 28.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
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
        .description = "Cuts armor. Volcano local forge.",
        .health = 18.0,
        .damage = 35.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
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
        .description = "Burst heal on defend. Capital holy craft.",
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
        .description = "Faster petal spin. Capital gear craft.",
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
        .description = "Reflects damage. Garden x Desert border rite.",
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
        .description = "Healing aura. Garden x Desert border rite.",
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
        .description = "A frosty armored barrier for defending",
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
        .description = "Hatches a spore ant to fight for you",
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
        .description = "Increases your movement speed while equipped",
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
        .description = "A protective shell with armor and strong poison resistance",
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
        .description = "Strengthens attacks and partially ignores armor",
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
        .description = "A compact coral cluster with minor poison resistance",
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
        .description = "Increases flower durability and poison resistance",
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
        .description = "A royal thorn that executes low-health foes with amplified damage",
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
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
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
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
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
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
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
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
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
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 115.0,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
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
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
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
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
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
        .description = "Poison + slow. Prism of three sovereign relics.",
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
    // ---- Ultra / Super / Unique ladders (×2 from Mythic each step) ----
    // Strong Unique Basic (appended after meme kUniqueBasic so craft prefers this id).
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 19440, .damage = 11664, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 9720, .damage = 5832, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Basic", .description = "A nice petal, not too strong but not too weak", .health = 4860, .damage = 2916, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUltra, .attributes = {} },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 1944, .damage = 45360, .radius = 7, .reload = 6.0, .count = 3, .rarity = RarityID::kUnique, .attributes = { .clump_radius = 10} },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 972, .damage = 22680, .radius = 7, .reload = 6.0, .count = 3, .rarity = RarityID::kSuper, .attributes = { .clump_radius = 10 } },
    { .name = "Stinger", .description = "It really hurts, but it's really fragile", .health = 486, .damage = 11340, .radius = 7, .reload = 6.0, .count = 3, .rarity = RarityID::kUltra, .attributes = { .clump_radius = 10} },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 3240, .damage = 3240, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kUnique, .attributes = { .secondary_reload = 1.0, .burst_heal = 9072, .defend_only = 1 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 1620, .damage = 1620, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kSuper, .attributes = { .secondary_reload = 1.0, .burst_heal = 4536, .defend_only = 1 } },
    { .name = "Rose", .description = "Its healing properties are amazing. Not so good at combat though", .health = 810, .damage = 810, .radius = 10, .reload = 3.5, .count = 1, .rarity = RarityID::kUltra, .attributes = { .secondary_reload = 1.0, .burst_heal = 2268, .defend_only = 1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 6480, .damage = 5184, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kUnique, .attributes = { .constant_heal = 8, .icon_angle = -1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 3240, .damage = 2592, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kSuper, .attributes = { .constant_heal = 7, .icon_angle = -1 } },
    { .name = "Leaf", .description = "Gathers energy from the sun to passively heal your flower", .health = 1620, .damage = 1296, .radius = 10, .reload = 1.0, .count = 1, .rarity = RarityID::kUltra, .attributes = { .constant_heal = 6, .icon_angle = -1 } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 3240, .damage = 3240, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kUnique, .attributes = { .poison_damage = { .damage = 6480.0, .time = 6.0 } } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 1620, .damage = 1620, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kSuper, .attributes = { .poison_damage = { .damage = 3240.0, .time = 6.0 } } },
    { .name = "Iris", .description = "Very poisonous, but takes a while to do its work", .health = 810, .damage = 810, .radius = 7, .reload = 5.0, .count = 1, .rarity = RarityID::kUltra, .attributes = { .poison_damage = { .damage = 1620.0, .time = 6.0 } } },
    { .name = "Wing", .description = "Orbits outward, then returns", .health = 3240, .damage = 3240, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUnique, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "Orbits outward, then returns", .health = 1620, .damage = 1620, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kSuper, .attributes = { .icon_angle = 1 } },
    { .name = "Wing", .description = "Orbits outward, then returns", .health = 810, .damage = 810, .radius = 10, .reload = 2.5, .count = 1, .rarity = RarityID::kUltra, .attributes = { .icon_angle = 1 } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 2592, .damage = 8640, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kUnique, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 1296, .damage = 4320, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kSuper, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Missile", .description = "You can actually shoot this one", .health = 648, .damage = 2160, .radius = 10, .reload = 3.0, .count = 1, .rarity = RarityID::kUltra, .attributes = { .secondary_reload = 0.85, .defend_only = 1, .icon_angle = 1, .rotation_style = PetalAttributes::kFollowRot } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kUnique, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kSuper, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Bubble", .description = "You can right click to pop it and propel your flower", .health = 1, .damage = 0, .radius = 12, .reload = 1.75, .count = 1, .rarity = RarityID::kUltra, .attributes = { .secondary_reload = 0.35, .defend_only = 1 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 1080, .damage = 1512, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kUnique, .attributes = { .extra_rotation_speed = 2.25} },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 540, .damage = 756, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kSuper, .attributes = { .extra_rotation_speed = 2.0 } },
    { .name = "Faster", .description = "It's so light it makes your other petals spin faster", .health = 270, .damage = 378, .radius = 7, .reload = 0.5, .count = 1, .rarity = RarityID::kUltra, .attributes = { .extra_rotation_speed = 1.75} },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 21600, .damage = 2160, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 10800, .damage = 1080, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Rock", .description = "Even more durable, but slower to recharge", .health = 5400, .damage = 540, .radius = 12, .reload = 7.5, .count = 1, .rarity = RarityID::kUltra, .attributes = {} },
    { .name = "Web", .description = "Plant sticky webs that snare foes when defending", .health = 2160, .damage = 1080, .radius = 10, .reload = 3.0, .count = 3, .rarity = RarityID::kUnique, .attributes = { .clump_radius = 10, .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Web", .description = "Plant sticky webs that snare foes when defending", .health = 1080, .damage = 540, .radius = 10, .reload = 3.0, .count = 3, .rarity = RarityID::kSuper, .attributes = { .clump_radius = 10, .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Web", .description = "Plant sticky webs that snare foes when defending", .health = 540, .damage = 270, .radius = 10, .reload = 3.0, .count = 3, .rarity = RarityID::kUltra, .attributes = { .clump_radius = 10, .secondary_reload = 0.5, .defend_only = 1 } },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 9720, .damage = 15552, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 4860, .damage = 7776, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Fast", .description = "Weaker than most petals, but reloads very quickly", .health = 2430, .damage = 3888, .radius = 7, .reload = 1.0, .count = 1, .rarity = RarityID::kUltra, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 38880, .damage = 38880, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 19440, .damage = 19440, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Heavy", .description = "Very resilient and deals more damage, but reloads very slowly", .health = 9720, .damage = 9720, .radius = 12, .reload = 4.5, .count = 1, .rarity = RarityID::kUltra, .attributes = {} },
    { .name = "Twin", .description = "Two light petals in one slot", .health = 3240, .damage = 5184, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kUnique, .attributes = {} },
    { .name = "Twin", .description = "Two light petals in one slot", .health = 1620, .damage = 2592, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kSuper, .attributes = {} },
    { .name = "Twin", .description = "Two light petals in one slot", .health = 810, .damage = 1296, .radius = 7, .reload = 1.0, .count = 2, .rarity = RarityID::kUltra, .attributes = {} },
    // ---- Ant Hell exclusive (anthell.txt) ----
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 22.0, .damage = 11.0, .radius = 8.0, .reload = 0.85, .count = 1,
        .rarity = RarityID::kUnusual, .attributes = {}
    },
    {
        .name = "Shell Plate",
        .description = "A single hexagonal carapace plate that shrugs off hits.",
        .health = 52.0, .damage = 8.0, .radius = 12.0, .reload = 3.5, .count = 1,
        .rarity = RarityID::kUnusual, .attributes = { .armor = 4 }
    },
    {
        .name = "Acid Droplet",
        .description = "A lone acid bead that poisons on contact",
        .health = 18.0, .damage = 8.0, .radius = 9.0, .reload = 3.5, .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = { .poison_damage = { .damage = 7.0, .time = 3.5 } }
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
        .attributes = { .extra_rotation_speed = 0.32 }
    },
    {
        .name = "Tunnel Stone",
        .description = "A packed nest stone with armor that resists knockback",
        .health = 95.0, .damage = 12.0, .radius = 13.0, .reload = 5.0, .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .mass = 80.0, .armor = 3 }
    },
    {
        .name = "Iron Mandible",
        .description = "Hardened mandible that amplifies your petal damage",
        .health = 40.0, .damage = 18.0, .radius = 10.0, .reload = 1.1, .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .extra_damage_factor = 1.15f }
    },
    {
        .name = "Hive Disc",
        .description = "A thick armored hive disc that also spins petals faster",
        .health = 140.0, .damage = 14.0, .radius = 13.0, .reload = 4.5, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 6, .extra_rotation_speed = 0.25 }
    },
    {
        .name = "Venom Cocoon",
        .description = "A sealed purple cocoon — tough shell, leaking venom.",
        .health = 90.0, .damage = 14.0, .radius = 12.0, .reload = 5.5, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 4, .poison_damage = { .damage = 9.0, .time = 4.5 } }
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
        .health = 55.0, .damage = 0.0, .radius = 12.0, .reload = 12.0, .count = 1,
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
        .description = "Amber colony core that heals and spins petals faster",
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
        .health = 3024.0, .damage = 1080.0, .radius = 15.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5, .aura_heal = 45.0, .aura_radius = 700.0,
            .aura_duration = 30.0, .defend_only = 1, .icon_angle = -0.35f
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
        .health = 756.0, .damage = 270.0, .radius = 15.0, .reload = 4.0, .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5, .aura_heal = 28.0, .aura_radius = 500.0,
            .aura_duration = 26.0, .defend_only = 1, .icon_angle = -0.35f
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
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 900.0,
        .damage = 1.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
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
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
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
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
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
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
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
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
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
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 47520.0,
        .damage = 1080.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
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
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Honey",
        .description = "A thick amber honeycomb chunk. Sticky, durable, and slow when thrown.",
        .health = 11880.0,
        .damage = 270.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.35f, .defend_only = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 40.0,
        .damage = 0.2,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kCommon,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 120.0,
        .damage = 0.4,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kUnusual,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 300.0,
        .damage = 0.6,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 600.0,
        .damage = 0.8,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 2700.0,
        .damage = 2.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 21600.0,
        .damage = 16.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 10800.0,
        .damage = 8.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
    },
    {
        .name = "Meteor",
        .description = "Throw while defending to plant a heavy meteor crater obstacle",
        .health = 5400.0,
        .damage = 4.0,
        .radius = 15.0,
        .reload = 20.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5f, .mass = 0.55f, .aura_duration = 18.0f,
            .defend_only = 1, .rotation_style = PetalAttributes::kNoRot
        }
    },
    // ---- Golden Leaf ladder (CD reduction; stacks multiplicatively) ----
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 36.0, .damage = 18.0, .radius = 10.0, .reload = 1.0, .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_reload_factor = 0.92f, .icon_angle = -1 } // -8%
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 108.0, .damage = 54.0, .radius = 10.0, .reload = 1.0, .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_reload_factor = 0.89f, .icon_angle = -1 } // -11%
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 324.0, .damage = 162.0, .radius = 10.0, .reload = 1.0, .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_reload_factor = 0.86f, .icon_angle = -1 } // -14%
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 648.0, .damage = 324.0, .radius = 10.0, .reload = 1.0, .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_reload_factor = 0.82f, .icon_angle = -1 } // -18%
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 1296.0, .damage = 648.0, .radius = 10.0, .reload = 1.0, .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_reload_factor = 0.78f, .icon_angle = -1 } // -22%
    },
    {
        .name = "Golden Leaf",
        .description = "Shortens all petal cooldowns. Multiple copies multiply together.",
        .health = 2592.0, .damage = 1296.0, .radius = 10.0, .reload = 1.0, .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_reload_factor = 0.74f, .icon_angle = -1 } // -26%
    },
    // ---- Auto-filled rarity ladders (gen_petal_ladders.py) ----
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 270.0,
        .damage = 96.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 3.5,
            .slow_inflict_seconds = 0.85,
            .extra_range = 42,
            .radial_pulse_damage = 28.0,
            .radial_pulse_radius = 140.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 540.0,
        .damage = 192.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 3.5,
            .slow_inflict_seconds = 0.85,
            .extra_range = 42,
            .radial_pulse_damage = 28.0,
            .radial_pulse_radius = 140.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 1080.0,
        .damage = 384.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 3.5,
            .slow_inflict_seconds = 0.85,
            .extra_range = 42,
            .radial_pulse_damage = 28.0,
            .radial_pulse_radius = 140.0
        }
    },
    {
        .name = "Abyssal Tide",
        .description = "Abyssal ring + slow. Sovereign relic of Tide Core and Blizzard Eye.",
        .health = 2160.0,
        .damage = 768.0,
        .radius = 24.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 3.5,
            .slow_inflict_seconds = 0.85,
            .extra_range = 42,
            .radial_pulse_damage = 28.0,
            .radial_pulse_radius = 140.0
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
        .attributes = {
            // Was 1.65 (+65%) — felt like a rocket. +30% still cuts map travel
            // time meaningfully without making chase / PvE feel broken.
            .move_speed_factor = 1.30f,
            .equipment = EquipmentFlags::kAccelerator
        }
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
        .attributes = {
            // Was 1.65 (+65%) — felt like a rocket. +30% still cuts map travel
            // time meaningfully without making chase / PvE feel broken.
            .move_speed_factor = 1.30f,
            .equipment = EquipmentFlags::kAccelerator
        }
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
        .attributes = {
            // Was 1.65 (+65%) — felt like a rocket. +30% still cuts map travel
            // time meaningfully without making chase / PvE feel broken.
            .move_speed_factor = 1.30f,
            .equipment = EquipmentFlags::kAccelerator
        }
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
        .attributes = {
            // Was 1.65 (+65%) — felt like a rocket. +30% still cuts map travel
            // time meaningfully without making chase / PvE feel broken.
            .move_speed_factor = 1.30f,
            .equipment = EquipmentFlags::kAccelerator
        }
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
        .attributes = {
            // Was 1.65 (+65%) — felt like a rocket. +30% still cuts map travel
            // time meaningfully without making chase / PvE feel broken.
            .move_speed_factor = 1.30f,
            .equipment = EquipmentFlags::kAccelerator
        }
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
        .attributes = {
            // Was 1.65 (+65%) — felt like a rocket. +30% still cuts map travel
            // time meaningfully without making chase / PvE feel broken.
            .move_speed_factor = 1.30f,
            .equipment = EquipmentFlags::kAccelerator
        }
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
        .attributes = {
            // Was 1.65 (+65%) — felt like a rocket. +30% still cuts map travel
            // time meaningfully without making chase / PvE feel broken.
            .move_speed_factor = 1.30f,
            .equipment = EquipmentFlags::kAccelerator
        }
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
        .attributes = { .poison_damage = { .damage = 21.0, .time = 10.5 } }
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
        .attributes = { .poison_damage = { .damage = 63.0, .time = 31.5 } }
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
        .attributes = { .poison_damage = { .damage = 189.0, .time = 94.5 } }
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
        .attributes = { .poison_damage = { .damage = 567.0, .time = 283.5 } }
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
        .attributes = { .poison_damage = { .damage = 1134.0, .time = 567.0 } }
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
        .attributes = { .poison_damage = { .damage = 2268.0, .time = 1134.0 } }
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
        .attributes = { .poison_damage = { .damage = 4536.0, .time = 2268.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 63.0,
        .damage = 12.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 6.0, .time = 18.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 189.0,
        .damage = 36.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 18.0, .time = 54.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 567.0,
        .damage = 108.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 54.0, .time = 162.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 1701.0,
        .damage = 324.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 162.0, .time = 486.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 3402.0,
        .damage = 648.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 324.0, .time = 972.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 6804.0,
        .damage = 1296.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 648.0, .time = 1944.0 } }
    },
    {
        .name = "Acid Flask",
        .description = "Long poison. Ant Hell acid craft.",
        .health = 13608.0,
        .damage = 2592.0,
        .radius = 13.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 1296.0, .time = 3888.0 } }
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
        .health = 33.0,
        .damage = 21.0,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
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
        .health = 99.0,
        .damage = 63.0,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
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
        .health = 198.0,
        .damage = 126.0,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
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
        .health = 396.0,
        .damage = 252.0,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
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
        .health = 792.0,
        .damage = 504.0,
        .radius = 14.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kVineBug
        }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 132.0,
        .damage = 18.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 13.5, .on_hit_heal = 2.0 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 396.0,
        .damage = 54.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 40.5, .on_hit_heal = 2.0 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 792.0,
        .damage = 108.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 81.0, .on_hit_heal = 2.0 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 1584.0,
        .damage = 216.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 162.0, .on_hit_heal = 2.0 }
    },
    {
        .name = "Anemone Bloom",
        .description = "Restores health and sips life on contact.",
        .health = 3168.0,
        .damage = 432.0,
        .radius = 19.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 324.0, .on_hit_heal = 2.0 }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 57.0,
        .damage = 63.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 171.0,
        .damage = 189.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 513.0,
        .damage = 567.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 1539.0,
        .damage = 1701.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 3078.0,
        .damage = 3402.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 6156.0,
        .damage = 6804.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Ant Fang",
        .description = "Bonus vs poisoned. Ant Hell local forge.",
        .health = 12312.0,
        .damage = 13608.0,
        .radius = 10.0,
        .reload = 2.4,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .bonus_vs_poisoned = 0.35f }
    },
    {
        .name = "Ant King",
        .description = "Two soldier ants. Sovereign relic of Hive Heart and Royal Seal.",
        .health = 345.0,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kMythic,
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
        .health = 690.0,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kUltra,
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
        .health = 1380.0,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kSuper,
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
        .health = 2760.0,
        .damage = 0.0,
        .radius = 24.0,
        .reload = 14.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 3.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt
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
        .rarity = RarityID::kMythic,
        .attributes = {
            .vision_factor = 0.6,
            .equipment = EquipmentFlags::kAntennae 
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
        .rarity = RarityID::kUltra,
        .attributes = {
            .vision_factor = 0.6,
            .equipment = EquipmentFlags::kAntennae 
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
        .rarity = RarityID::kSuper,
        .attributes = {
            .vision_factor = 0.6,
            .equipment = EquipmentFlags::kAntennae 
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
        .rarity = RarityID::kUnique,
        .attributes = {
            .vision_factor = 0.6,
            .equipment = EquipmentFlags::kAntennae 
        }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 54.0,
        .damage = 30.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .poison_damage = { .damage = 9.0, .time = 6.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 162.0,
        .damage = 90.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 27.0, .time = 18.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 486.0,
        .damage = 270.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 81.0, .time = 54.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 1458.0,
        .damage = 810.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 243.0, .time = 162.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 2916.0,
        .damage = 1620.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 486.0, .time = 324.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 5832.0,
        .damage = 3240.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 972.0, .time = 648.0 } }
    },
    {
        .name = "Bee Sting",
        .description = "Brief poison sting. Garden bee-rite weapon.",
        .health = 11664.0,
        .damage = 6480.0,
        .radius = 9.0,
        .reload = 1.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 1944.0, .time = 1296.0 } }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 36.0,
        .damage = 45.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .armor = 8,
            .icon_angle = 1
        }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 72.0,
        .damage = 90.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .armor = 8,
            .icon_angle = 1
        }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 144.0,
        .damage = 180.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .armor = 8,
            .icon_angle = 1
        }
    },
    {
        .name = "Bone",
        .description = "A sturdy bone that adds petal armor",
        .health = 288.0,
        .damage = 360.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .armor = 8,
            .icon_angle = 1
        }
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
        .health = 45.0,
        .damage = 15.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 10,
            .extra_health = 40,
        }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 90.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 10,
            .extra_health = 40,
        }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 180.0,
        .damage = 60.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 10,
            .extra_health = 40,
        }
    },
    {
        .name = "Cactus",
        .description = "Not very strong, but somehow increases your maximum health",
        .health = 360.0,
        .damage = 120.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 10,
            .extra_health = 40,
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 249.0,
        .damage = 93.0,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .poison_damage = { .damage = 21.0, .time = 9.0 },
            .slow_inflict_seconds = 0.75
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 498.0,
        .damage = 186.0,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .poison_damage = { .damage = 42.0, .time = 18.0 },
            .slow_inflict_seconds = 0.75
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 996.0,
        .damage = 372.0,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .poison_damage = { .damage = 84.0, .time = 36.0 },
            .slow_inflict_seconds = 0.75
        }
    },
    {
        .name = "Chaos Prism",
        .description = "Poison + slow. Prism of three sovereign relics.",
        .health = 1992.0,
        .damage = 744.0,
        .radius = 22.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .poison_damage = { .damage = 168.0, .time = 72.0 },
            .slow_inflict_seconds = 0.75
        }
    },
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 66.0,
        .damage = 33.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 198.0,
        .damage = 99.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 594.0,
        .damage = 297.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 1782.0,
        .damage = 891.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 3564.0,
        .damage = 1782.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 7128.0,
        .damage = 3564.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Chitin Needle",
        .description = "A straight chitin spike — thinner and longer than an ant fang.",
        .health = 14256.0,
        .damage = 7128.0,
        .radius = 8.0,
        .reload = 0.85,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 240.0,
        .damage = 42.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.5,
            .slow_inflict_seconds = 2.0,
            .aura_radius = 360.0,
            .aura_duration = 6.0,
            .aura_slow_seconds = 1.2,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 480.0,
        .damage = 84.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5,
            .slow_inflict_seconds = 2.0,
            .aura_radius = 720.0,
            .aura_duration = 6.0,
            .aura_slow_seconds = 1.2,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 960.0,
        .damage = 168.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.5,
            .slow_inflict_seconds = 2.0,
            .aura_radius = 1440.0,
            .aura_duration = 6.0,
            .aura_slow_seconds = 1.2,
            .defend_only = 1
        }
    },
    {
        .name = "Chrono Vortex",
        .description = "Defend-throw slow aura. Relic binding Sandstorm Eye, Sovereign Core, Tide Core.",
        .health = 1920.0,
        .damage = 336.0,
        .radius = 23.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5,
            .slow_inflict_seconds = 2.0,
            .aura_radius = 2880.0,
            .aura_duration = 6.0,
            .aura_slow_seconds = 1.2,
            .defend_only = 1
        }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 174.0,
        .damage = 27.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .move_speed_factor = 1.08f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 522.0,
        .damage = 81.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .move_speed_factor = 1.08f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 1044.0,
        .damage = 162.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .move_speed_factor = 1.08f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 2088.0,
        .damage = 324.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .move_speed_factor = 1.08f }
    },
    {
        .name = "Colony Core",
        .description = "Increases your movement speed while equipped",
        .health = 4176.0,
        .damage = 648.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .move_speed_factor = 1.08f }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 360.0,
        .damage = 48.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 1.0,
            .constant_heal = 12.0,
            .burst_heal = 12.0,
            .extra_rotation_speed = 0.4
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 720.0,
        .damage = 96.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 1.0,
            .constant_heal = 24.0,
            .burst_heal = 12.0,
            .extra_rotation_speed = 0.4
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 1440.0,
        .damage = 192.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 1.0,
            .constant_heal = 48.0,
            .burst_heal = 12.0,
            .extra_rotation_speed = 0.4
        }
    },
    {
        .name = "Colony Heart",
        .description = "Amber colony core that heals and spins petals faster",
        .health = 2880.0,
        .damage = 384.0,
        .radius = 14.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 1.0,
            .constant_heal = 96.0,
            .burst_heal = 12.0,
            .extra_rotation_speed = 0.4
        }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 147.0,
        .damage = 42.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 20, .poison_armor = 6 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 441.0,
        .damage = 126.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 20, .poison_armor = 6 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 882.0,
        .damage = 252.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 20, .poison_armor = 6 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 1764.0,
        .damage = 504.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 20, .poison_armor = 6 }
    },
    {
        .name = "Coral Hive",
        .description = "A compact coral cluster with minor poison resistance",
        .health = 3528.0,
        .damage = 1008.0,
        .radius = 17.0,
        .reload = 4.8,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 20, .poison_armor = 6 }
    },
    {
        .name = "Corn",
        .description = "Takes a long time to spawn, but has a lot of health",
        .health = 1500.0,
        .damage = 7.5,
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
        .health = 4500.0,
        .damage = 22.5,
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
        .health = 9000.0,
        .damage = 45.0,
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
        .health = 18000.0,
        .damage = 90.0,
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
        .health = 36000.0,
        .damage = 180.0,
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
        .health = 138.0,
        .damage = 27.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 18.0, .time = 15.0 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 414.0,
        .damage = 81.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 54.0, .time = 45.0 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 828.0,
        .damage = 162.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 108.0, .time = 90.0 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 1656.0,
        .damage = 324.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 216.0, .time = 180.0 } }
    },
    {
        .name = "Corrode Vine",
        .description = "Inflicts strong, lingering poison on contact.",
        .health = 3312.0,
        .damage = 648.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 432.0, .time = 360.0 } }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 165.0,
        .damage = 15.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .damage_reflection = 0.08, .extra_health = 12 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 495.0,
        .damage = 45.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .damage_reflection = 0.08, .extra_health = 12 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 1485.0,
        .damage = 135.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.08, .extra_health = 12 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 4455.0,
        .damage = 405.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.08, .extra_health = 12 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 8910.0,
        .damage = 810.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.08, .extra_health = 12 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 17820.0,
        .damage = 1620.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.08, .extra_health = 12 }
    },
    {
        .name = "Craft Cactus",
        .description = "Reflects a little damage. Desert carapace craft.",
        .health = 35640.0,
        .damage = 3240.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.08, .extra_health = 12 }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 54.0,
        .damage = 36.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            // Flat add on flattened body curve (~+28% at L100 body≈89).
            .extra_body_damage = 25,
            .equipment = EquipmentFlags::kCutter
        }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 162.0,
        .damage = 108.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            // Flat add on flattened body curve (~+28% at L100 body≈89).
            .extra_body_damage = 25,
            .equipment = EquipmentFlags::kCutter
        }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 324.0,
        .damage = 216.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            // Flat add on flattened body curve (~+28% at L100 body≈89).
            .extra_body_damage = 25,
            .equipment = EquipmentFlags::kCutter
        }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 648.0,
        .damage = 432.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            // Flat add on flattened body curve (~+28% at L100 body≈89).
            .extra_body_damage = 25,
            .equipment = EquipmentFlags::kCutter
        }
    },
    {
        .name = "Cutter",
        .description = "Increases the flower's body damage",
        .health = 1296.0,
        .damage = 864.0,
        .radius = 40.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            // Flat add on flattened body curve (~+28% at L100 body≈89).
            .extra_body_damage = 25,
            .equipment = EquipmentFlags::kCutter
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 30.0,
        .damage = 30.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .dandelion_inflict_seconds = 10.0f,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 90.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .dandelion_inflict_seconds = 10.0f,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 270.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .dandelion_inflict_seconds = 10.0f,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 540.0,
        .damage = 540.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .dandelion_inflict_seconds = 10.0f,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 1080.0,
        .damage = 1080.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .dandelion_inflict_seconds = 10.0f,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Dandelion",
        .description = "Prevents healing on units it hits for a short time",
        .health = 2160.0,
        .damage = 2160.0,
        .radius = 10.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .dandelion_inflict_seconds = 10.0f,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 117.0,
        .damage = 87.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.12f, .bonus_vs_slowed = 0.25f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 351.0,
        .damage = 261.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.12f, .bonus_vs_slowed = 0.25f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 702.0,
        .damage = 522.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.12f, .bonus_vs_slowed = 0.25f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 1404.0,
        .damage = 1044.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.12f, .bonus_vs_slowed = 0.25f }
    },
    {
        .name = "Deep Frost Pearl",
        .description = "A dense frost pearl that hits harder against slowed foes.",
        .health = 2808.0,
        .damage = 2088.0,
        .radius = 16.0,
        .reload = 4.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.12f, .bonus_vs_slowed = 0.25f }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 258.0,
        .damage = 30.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 4.5, .extra_health = 22 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 774.0,
        .damage = 90.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 13.5, .extra_health = 22 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 1548.0,
        .damage = 180.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 27.0, .extra_health = 22 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 3096.0,
        .damage = 360.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 54.0, .extra_health = 22 }
    },
    {
        .name = "Desert Root",
        .description = "A tough root that slowly restores health.",
        .health = 6192.0,
        .damage = 720.0,
        .radius = 20.0,
        .reload = 5.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 108.0, .extra_health = 22 }
    },
    {
        .name = "Egg",
        .description = "Something interesting might pop out of this",
        .health = 150.0,
        .damage = 3.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
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
        .health = 450.0,
        .damage = 9.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kMythic,
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
        .health = 900.0,
        .damage = 18.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUltra,
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
        .health = 1800.0,
        .damage = 36.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kSuper,
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
        .health = 3600.0,
        .damage = 72.0,
        .radius = 15.0,
        .reload = 1.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .secondary_reload = 2.5,
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
        .attributes = { .armor = 5 }
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
        .attributes = { .armor = 5 }
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
        .attributes = { .armor = 5 }
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
        .attributes = { .armor = 5 }
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
        .attributes = { .armor = 5 }
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
        .attributes = { .armor = 5 }
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
        .attributes = { .armor = 5 }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 48.0,
        .damage = 84.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 144.0,
        .damage = 252.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 432.0,
        .damage = 756.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 1296.0,
        .damage = 2268.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 2592.0,
        .damage = 4536.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 5184.0,
        .damage = 9072.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Fireball",
        .description = "Burns on contact. Volcano local forge.",
        .health = 10368.0,
        .damage = 18144.0,
        .radius = 13.0,
        .reload = 3.3,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .burn_damage = { .damage = 4.0, .time = 3.0 } }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 132.0,
        .damage = 69.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.25, .extra_range = 8 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 396.0,
        .damage = 207.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.25, .extra_range = 8 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 792.0,
        .damage = 414.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.25, .extra_range = 8 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 1584.0,
        .damage = 828.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.25, .extra_range = 8 }
    },
    {
        .name = "Flame Gear",
        .description = "Speeds up petal rotation and slightly extends its reach.",
        .health = 3168.0,
        .damage = 1656.0,
        .radius = 18.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.25, .extra_range = 8 }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 144.0,
        .damage = 96.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_damage_factor = 1.08f, .armor_penetration = 0.25f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 432.0,
        .damage = 288.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.08f, .armor_penetration = 0.25f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 864.0,
        .damage = 576.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.08f, .armor_penetration = 0.25f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 1728.0,
        .damage = 1152.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.08f, .armor_penetration = 0.25f }
    },
    {
        .name = "Forged Holy Sword",
        .description = "Strengthens attacks and partially ignores armor",
        .health = 3456.0,
        .damage = 2304.0,
        .radius = 15.0,
        .reload = 4.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.08f, .armor_penetration = 0.25f }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 216.0,
        .damage = 15.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 8, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 648.0,
        .damage = 45.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 8, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 1296.0,
        .damage = 90.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 8, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 2592.0,
        .damage = 180.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 8, .defend_only = 1 }
    },
    {
        .name = "Frost Bubble",
        .description = "A frosty armored barrier for defending",
        .health = 5184.0,
        .damage = 360.0,
        .radius = 21.0,
        .reload = 4.6,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 8, .defend_only = 1 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 237.0,
        .damage = 21.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 32, .poison_armor = 8 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 711.0,
        .damage = 63.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 32, .poison_armor = 8 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 1422.0,
        .damage = 126.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 32, .poison_armor = 8 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 2844.0,
        .damage = 252.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 32, .poison_armor = 8 }
    },
    {
        .name = "Frost Crown",
        .description = "Increases flower durability and poison resistance",
        .health = 5688.0,
        .damage = 504.0,
        .radius = 21.0,
        .reload = 6.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 32, .poison_armor = 8 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 198.0,
        .damage = 84.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 12, .slow_inflict_seconds = 0.3 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 594.0,
        .damage = 252.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 12, .slow_inflict_seconds = 0.3 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 1188.0,
        .damage = 504.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 12, .slow_inflict_seconds = 0.3 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 2376.0,
        .damage = 1008.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 12, .slow_inflict_seconds = 0.3 }
    },
    {
        .name = "Frost Knight Sword",
        .description = "A durable frost sword that briefly slows enemies.",
        .health = 4752.0,
        .damage = 2016.0,
        .radius = 15.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 12, .slow_inflict_seconds = 0.3 }
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost starfish provide sustained contact pressure.",
        .health = 84.0,
        .damage = 45.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost starfish provide sustained contact pressure.",
        .health = 252.0,
        .damage = 135.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost starfish provide sustained contact pressure.",
        .health = 504.0,
        .damage = 270.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost starfish provide sustained contact pressure.",
        .health = 1008.0,
        .damage = 540.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Frost Starfish",
        .description = "Three frost starfish provide sustained contact pressure.",
        .health = 2016.0,
        .damage = 1080.0,
        .radius = 13.0,
        .reload = 2.6,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 72.0,
        .damage = 60.0,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 216.0,
        .damage = 180.0,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 432.0,
        .damage = 360.0,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 864.0,
        .damage = 720.0,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Glass Storm",
        .description = "Launches a projectile that splits into glass shards.",
        .health = 1728.0,
        .damage = 1440.0,
        .radius = 13.0,
        .reload = 5.2,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .defend_only = 1,
            .split_projectile = 1
        }
    },
    {
        .name = "Guard Egg",
        .description = "Hatches a wandering royal guard",
        .health = 33.0,
        .damage = 19.5,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
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
        .health = 99.0,
        .damage = 58.5,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kMythic,
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
        .health = 198.0,
        .damage = 117.0,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUltra,
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
        .health = 396.0,
        .damage = 234.0,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kSuper,
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
        .health = 792.0,
        .damage = 468.0,
        .radius = 14.0,
        .reload = 3.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 2.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kWanderingGuard
        }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 72.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .constant_heal = 6.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 216.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .constant_heal = 18.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 648.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 54.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 1944.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 162.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 3888.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 324.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 7776.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 648.0, .defend_only = 1 }
    },
    {
        .name = "Healing Bloom",
        .description = "Gentle heal while equipped. Garden bloom rite.",
        .health = 15552.0,
        .damage = 0.0,
        .radius = 13.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 1296.0, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 285.0,
        .damage = 0.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 21.0, .extra_health = 35, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 570.0,
        .damage = 0.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 42.0, .extra_health = 35, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 1140.0,
        .damage = 0.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 84.0, .extra_health = 35, .defend_only = 1 }
    },
    {
        .name = "Heart of Nature",
        .description = "Durable + heal. Sovereign relic of Ancient Heart and Heartwood.",
        .health = 2280.0,
        .damage = 0.0,
        .radius = 23.0,
        .reload = 9.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 168.0, .extra_health = 35, .defend_only = 1 }
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 600.0,
        .damage = 30.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 1800.0,
        .damage = 90.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 3600.0,
        .damage = 180.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 7200.0,
        .damage = 360.0,
        .radius = 12.0,
        .reload = 15.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Heaviest",
        .description = "This thing is so heavy that nothing gets in the way",
        .health = 14400.0,
        .damage = 720.0,
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
        .attributes = { .armor = 6, .extra_rotation_speed = 0.25 }
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
        .attributes = { .armor = 6, .extra_rotation_speed = 0.25 }
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
        .attributes = { .armor = 6, .extra_rotation_speed = 0.25 }
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
        .attributes = { .armor = 6, .extra_rotation_speed = 0.25 }
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
        .attributes = { .armor = 6, .extra_rotation_speed = 0.25 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 138.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 36.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 414.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 36.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 828.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 36.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 1656.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 36.0, .defend_only = 1 }
    },
    {
        .name = "Holy Flame Core",
        .description = "Releases a powerful burst of healing.",
        .health = 3312.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 36.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 90.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 20.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 270.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 20.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 810.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 20.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 1620.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 20.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 3240.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 20.0, .defend_only = 1 }
    },
    {
        .name = "Holy Orb",
        .description = "Burst heal on defend. Capital holy craft.",
        .health = 6480.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 20.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 63.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 16.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 189.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 16.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 378.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 16.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 756.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 16.0, .defend_only = 1 }
    },
    {
        .name = "Holy Snowflake",
        .description = "Two fragile snowflakes provide bursts of healing.",
        .health = 1512.0,
        .damage = 0.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .secondary_reload = 1.0, .burst_heal = 16.0, .defend_only = 1 }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 135.0,
        .damage = 36.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .constant_heal = 4.5,
            .poison_damage = { .damage = 9.0, .time = 9.0 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 405.0,
        .damage = 108.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .constant_heal = 13.5,
            .poison_damage = { .damage = 27.0, .time = 27.0 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 810.0,
        .damage = 216.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .constant_heal = 27.0,
            .poison_damage = { .damage = 54.0, .time = 54.0 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 1620.0,
        .damage = 432.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .constant_heal = 54.0,
            .poison_damage = { .damage = 108.0, .time = 108.0 }
        }
    },
    {
        .name = "Holy Spore",
        .description = "Inflicts poison while slowly restoring your health.",
        .health = 3240.0,
        .damage = 864.0,
        .radius = 17.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .constant_heal = 108.0,
            .poison_damage = { .damage = 216.0, .time = 216.0 }
        }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 186.0,
        .damage = 6.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor = 10, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 558.0,
        .damage = 18.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 10, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 1674.0,
        .damage = 54.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 10, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 3348.0,
        .damage = 108.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 10, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 6696.0,
        .damage = 216.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 10, .defend_only = 1 }
    },
    {
        .name = "Ice Shield",
        .description = "High petal armor. Snow local forge.",
        .health = 13392.0,
        .damage = 432.0,
        .radius = 19.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 10, .defend_only = 1 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 60.0,
        .damage = 57.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 0.35 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 180.0,
        .damage = 171.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 0.35 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 540.0,
        .damage = 513.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 0.35 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 1620.0,
        .damage = 1539.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 0.35 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 3240.0,
        .damage = 3078.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 0.35 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 6480.0,
        .damage = 6156.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 0.35 }
    },
    {
        .name = "Ice Spike",
        .description = "Slows on hit. Snow traveler's spike.",
        .health = 12960.0,
        .damage = 12312.0,
        .radius = 9.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 0.35 }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 144.0,
        .damage = 48.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 3.0,
            .slow_inflict_seconds = 0.45,
            .extra_range = 25,
            .radial_pulse_damage = 18.0,
            .radial_pulse_radius = 110.0
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 432.0,
        .damage = 144.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 3.0,
            .slow_inflict_seconds = 0.45,
            .extra_range = 25,
            .radial_pulse_damage = 18.0,
            .radial_pulse_radius = 110.0
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 864.0,
        .damage = 288.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 3.0,
            .slow_inflict_seconds = 0.45,
            .extra_range = 25,
            .radial_pulse_damage = 18.0,
            .radial_pulse_radius = 110.0
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 1728.0,
        .damage = 576.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 3.0,
            .slow_inflict_seconds = 0.45,
            .extra_range = 25,
            .radial_pulse_damage = 18.0,
            .radial_pulse_radius = 110.0
        }
    },
    {
        .name = "Ice Tide Ring",
        .description = "Extends orbit, slows on contact, and pulses an icy ring when attacking.",
        .health = 3456.0,
        .damage = 1152.0,
        .radius = 18.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 3.0,
            .slow_inflict_seconds = 0.45,
            .extra_range = 25,
            .radial_pulse_damage = 18.0,
            .radial_pulse_radius = 110.0
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 336.0,
        .damage = 171.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_health = 25,
            .burn_damage = { .damage = 8.0, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 672.0,
        .damage = 342.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_health = 25,
            .burn_damage = { .damage = 8.0, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 1344.0,
        .damage = 684.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_health = 25,
            .burn_damage = { .damage = 8.0, .time = 4.0 }
        }
    },
    {
        .name = "Infernal Core",
        .description = "Burns on contact. Sovereign relic of Leyline Core.",
        .health = 2688.0,
        .damage = 1368.0,
        .radius = 24.0,
        .reload = 9.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_health = 25,
            .burn_damage = { .damage = 8.0, .time = 4.0 }
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
        .attributes = { .armor = 3 }
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
        .attributes = { .armor = 3 }
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
        .attributes = { .armor = 3 }
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
        .attributes = { .armor = 3 }
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
        .attributes = { .armor = 3 }
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
        .attributes = { .armor = 3 }
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
        .attributes = { .armor = 3 }
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
        .attributes = { .armor = 3 }
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
        .attributes = { .extra_damage_factor = 1.15f }
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
        .attributes = { .extra_damage_factor = 1.15f }
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
        .attributes = { .extra_damage_factor = 1.15f }
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
        .attributes = { .extra_damage_factor = 1.15f }
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
        .attributes = { .extra_damage_factor = 1.15f }
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
        .attributes = { .extra_damage_factor = 1.15f }
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
        .health = 39.0,
        .damage = 78.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_damage_factor = 1.15f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 78.0,
        .damage = 156.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_damage_factor = 1.15f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 156.0,
        .damage = 312.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_damage_factor = 1.15f
        }
    },
    {
        .name = "King's Oath",
        .description = "Amplifies petal damage; surges harder when your flower is low on health",
        .health = 312.0,
        .damage = 624.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_damage_factor = 1.15f
        }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 22.5,
        .damage = 27.0,
        .radius = 12.0,
        .reload = 1.875,
        .count = 3,
        .rarity = RarityID::kUnusual,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 67.5,
        .damage = 81.0,
        .radius = 12.0,
        .reload = 1.875,
        .count = 3,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 202.5,
        .damage = 243.0,
        .radius = 12.0,
        .reload = 1.875,
        .count = 3,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 607.5,
        .damage = 729.0,
        .radius = 12.0,
        .reload = 1.875,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 1822.5,
        .damage = 2187.0,
        .radius = 12.0,
        .reload = 1.875,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 3645.0,
        .damage = 4374.0,
        .radius = 12.0,
        .reload = 1.875,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 7290.0,
        .damage = 8748.0,
        .radius = 12.0,
        .reload = 1.875,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Leaf Blade",
        .description = "Cuts quickly. Pathblade of Garden travelers.",
        .health = 14580.0,
        .damage = 17496.0,
        .radius = 12.0,
        .reload = 1.875,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 10 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 165.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .constant_heal = 9.0, .extra_health = 15, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 495.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .constant_heal = 27.0, .extra_health = 15, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 990.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .constant_heal = 54.0, .extra_health = 15, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 1980.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .constant_heal = 108.0, .extra_health = 15, .defend_only = 1 }
    },
    {
        .name = "Life Lotus",
        .description = "Gradually heals you and increases petal durability.",
        .health = 3960.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .constant_heal = 216.0, .extra_health = 15, .defend_only = 1 }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 66.0,
        .damage = 15.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .poison_armor = 12,
            .icon_angle = 0.1
        }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 198.0,
        .damage = 45.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .poison_armor = 12,
            .icon_angle = 0.1
        }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 396.0,
        .damage = 90.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .poison_armor = 12,
            .icon_angle = 0.1
        }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 792.0,
        .damage = 180.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .poison_armor = 12,
            .icon_angle = 0.1
        }
    },
    {
        .name = "Lotus",
        .description = "Absorbs some poison damage taken by the flower",
        .health = 1584.0,
        .damage = 360.0,
        .radius = 12.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .poison_armor = 12,
            .icon_angle = 0.1
        }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 177.0,
        .damage = 72.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 10, .splash_damage = 12.0, .splash_radius = 70.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 531.0,
        .damage = 216.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 10, .splash_damage = 12.0, .splash_radius = 70.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 1593.0,
        .damage = 648.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 10, .splash_damage = 12.0, .splash_radius = 70.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 3186.0,
        .damage = 1296.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 10, .splash_damage = 12.0, .splash_radius = 70.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 6372.0,
        .damage = 2592.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 10, .splash_damage = 12.0, .splash_radius = 70.0 }
    },
    {
        .name = "Magma Core",
        .description = "Molten splash. Volcano local forge.",
        .health = 12744.0,
        .damage = 5184.0,
        .radius = 18.0,
        .reload = 6.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 10, .splash_damage = 12.0, .splash_radius = 70.0 }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 24.0,
        .damage = 15.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {
            .magnet_radius = 1800.f,
            .equipment = EquipmentFlags::kMagnet
        }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 72.0,
        .damage = 45.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .magnet_radius = 1800.f,
            .equipment = EquipmentFlags::kMagnet
        }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 216.0,
        .damage = 135.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .magnet_radius = 1800.f,
            .equipment = EquipmentFlags::kMagnet
        }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 648.0,
        .damage = 405.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .magnet_radius = 1800.f,
            .equipment = EquipmentFlags::kMagnet
        }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 1296.0,
        .damage = 810.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .magnet_radius = 1800.f,
            .equipment = EquipmentFlags::kMagnet
        }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 2592.0,
        .damage = 1620.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .magnet_radius = 1800.f,
            .equipment = EquipmentFlags::kMagnet
        }
    },
    {
        .name = "Magnet",
        .description = "Pulls nearby petal drops toward your flower",
        .health = 5184.0,
        .damage = 3240.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .magnet_radius = 1800.f,
            .equipment = EquipmentFlags::kMagnet
        }
    },
    {
        .name = "Mantis Claw",
        .description = "A scythe-like claw for heavy strikes",
        .health = 27.0,
        .damage = 63.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Mantis Claw",
        .description = "A scythe-like claw for heavy strikes",
        .health = 81.0,
        .damage = 189.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Mantis Claw",
        .description = "A scythe-like claw for heavy strikes",
        .health = 162.0,
        .damage = 378.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Mantis Claw",
        .description = "A scythe-like claw for heavy strikes",
        .health = 324.0,
        .damage = 756.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Mantis Claw",
        .description = "A scythe-like claw for heavy strikes",
        .health = 648.0,
        .damage = 1512.0,
        .radius = 17.0,
        .reload = 4.75,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 120.0,
        .damage = 57.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_rotation_speed = 0.18 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 360.0,
        .damage = 171.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_rotation_speed = 0.18 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 1080.0,
        .damage = 513.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_rotation_speed = 0.18 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 2160.0,
        .damage = 1026.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_rotation_speed = 0.18 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 4320.0,
        .damage = 2052.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_rotation_speed = 0.18 }
    },
    {
        .name = "Mech Gear",
        .description = "Faster petal spin. Capital gear craft.",
        .health = 8640.0,
        .damage = 4104.0,
        .radius = 16.0,
        .reload = 3.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_rotation_speed = 0.18 }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 102.0,
        .damage = 51.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_rotation_speed = 0.14,
            .poison_damage = { .damage = 9.0, .time = 6.0 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 306.0,
        .damage = 153.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_rotation_speed = 0.14,
            .poison_damage = { .damage = 27.0, .time = 18.0 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 612.0,
        .damage = 306.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_rotation_speed = 0.14,
            .poison_damage = { .damage = 54.0, .time = 36.0 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 1224.0,
        .damage = 612.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_rotation_speed = 0.14,
            .poison_damage = { .damage = 108.0, .time = 72.0 }
        }
    },
    {
        .name = "Mech Poison Bee",
        .description = "Fast-spinning mechanical bees inflict short poison.",
        .health = 2448.0,
        .damage = 1224.0,
        .radius = 14.0,
        .reload = 3.4,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_rotation_speed = 0.14,
            .poison_damage = { .damage = 216.0, .time = 144.0 }
        }
    },
    {
        .name = "Molten Sand Blade",
        .description = "A fragile molten blade that delivers heavy strikes.",
        .health = 78.0,
        .damage = 126.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Molten Sand Blade",
        .description = "A fragile molten blade that delivers heavy strikes.",
        .health = 234.0,
        .damage = 378.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Molten Sand Blade",
        .description = "A fragile molten blade that delivers heavy strikes.",
        .health = 468.0,
        .damage = 756.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Molten Sand Blade",
        .description = "A fragile molten blade that delivers heavy strikes.",
        .health = 936.0,
        .damage = 1512.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Molten Sand Blade",
        .description = "A fragile molten blade that delivers heavy strikes.",
        .health = 1872.0,
        .damage = 3024.0,
        .radius = 14.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Observer",
        .description = "The one who sees all",
        .health = 0.0,
        .damage = 0.0,
        .radius = 12.5,
        .reload = 0.0,
        .count = 0,
        .rarity = RarityID::kUltra,
        .attributes = {
            .vision_factor = 0.25,
            .equipment = EquipmentFlags::kObserver
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
        .rarity = RarityID::kSuper,
        .attributes = {
            .vision_factor = 0.25,
            .equipment = EquipmentFlags::kObserver
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
        .rarity = RarityID::kUnique,
        .attributes = {
            .vision_factor = 0.25,
            .equipment = EquipmentFlags::kObserver
        }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 54.0,
        .damage = 105.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 162.0,
        .damage = 315.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 486.0,
        .damage = 945.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 972.0,
        .damage = 1890.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 1944.0,
        .damage = 3780.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Obsidian Blade",
        .description = "Cuts armor. Volcano local forge.",
        .health = 3888.0,
        .damage = 7560.0,
        .radius = 12.0,
        .reload = 3.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor_penetration = 0.5f }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 336.0,
        .damage = 9.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.28, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 1008.0,
        .damage = 27.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.28, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 2016.0,
        .damage = 54.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.28, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 4032.0,
        .damage = 108.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.28, .defend_only = 1 }
    },
    {
        .name = "Obsidian King Shield",
        .description = "A massive shield that reflects damage when struck.",
        .health = 8064.0,
        .damage = 216.0,
        .radius = 24.0,
        .reload = 7.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.28, .defend_only = 1 }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 15.0,
        .damage = 6.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .poison_damage = {
                .damage = 60.0,
                .time = 1.5
            },
            .defend_only = 1,
            .split_projectile = 1,
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 45.0,
        .damage = 18.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kMythic,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .poison_damage = {
                .damage = 180.0,
                .time = 4.5
            },
            .defend_only = 1,
            .split_projectile = 1,
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 90.0,
        .damage = 36.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUltra,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .poison_damage = {
                .damage = 360.0,
                .time = 9.0
            },
            .defend_only = 1,
            .split_projectile = 1,
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 180.0,
        .damage = 72.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kSuper,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .poison_damage = {
                .damage = 720.0,
                .time = 18.0
            },
            .defend_only = 1,
            .split_projectile = 1,
        }
    },
    {
        .name = "Peas",
        .description = "4 in 1 deal, now with a secret ingredient: poison",
        .health = 360.0,
        .damage = 144.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 4,
        .rarity = RarityID::kUnique,
        .attributes = {
            .clump_radius = 8,
            .secondary_reload = 0.5,
            .poison_damage = {
                .damage = 1440.0,
                .time = 36.0
            },
            .defend_only = 1,
            .split_projectile = 1,
        }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 159.0,
        .damage = 51.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 4, .extra_range = 30 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 477.0,
        .damage = 153.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 4, .extra_range = 30 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 954.0,
        .damage = 306.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 4, .extra_range = 30 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 1908.0,
        .damage = 612.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 4, .extra_range = 30 }
    },
    {
        .name = "Permafrost Gear",
        .description = "Extends petal orbit range and adds minor armor.",
        .health = 3816.0,
        .damage = 1224.0,
        .radius = 18.0,
        .reload = 5.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 4, .extra_range = 30 }
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
            .extra_damage_factor = 1.12f
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
            .extra_damage_factor = 1.12f
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
            .extra_damage_factor = 1.12f
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
            .extra_damage_factor = 1.12f
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
            .extra_damage_factor = 1.12f
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
            .extra_damage_factor = 1.12f
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
        .attributes = { .extra_rotation_speed = 0.32 }
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
        .attributes = { .extra_rotation_speed = 0.32 }
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
        .attributes = { .extra_rotation_speed = 0.32 }
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
        .attributes = { .extra_rotation_speed = 0.32 }
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
        .attributes = { .extra_rotation_speed = 0.32 }
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
        .attributes = { .extra_rotation_speed = 0.32 }
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
            .slow_inflict_seconds = 2,
            .poison_damage = {
                .damage = 15.0,
                .time = 3.0
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
            .slow_inflict_seconds = 2,
            .poison_damage = {
                .damage = 45.0,
                .time = 9.0
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
            .slow_inflict_seconds = 2,
            .poison_damage = {
                .damage = 135.0,
                .time = 27.0
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
            .slow_inflict_seconds = 2,
            .poison_damage = {
                .damage = 270.0,
                .time = 54.0
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
            .slow_inflict_seconds = 2,
            .poison_damage = {
                .damage = 540.0,
                .time = 108.0
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
            .slow_inflict_seconds = 2,
            .poison_damage = {
                .damage = 1080.0,
                .time = 216.0
            },
            .icon_angle = 0.7
        }
    },
    {
        .name = "Poison Ant Nest",
        .description = "Hatches a spore ant to fight for you",
        .health = 168.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
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
        .health = 504.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kMythic,
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
        .health = 1008.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUltra,
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
        .health = 2016.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kSuper,
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
        .health = 4032.0,
        .damage = 0.0,
        .radius = 20.0,
        .reload = 14.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 3.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSporeAnt
        }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 27.0,
        .damage = 24.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = { .clump_radius = 9, .poison_damage = { .damage = 18.0, .time = 9.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 81.0,
        .damage = 72.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = { .clump_radius = 9, .poison_damage = { .damage = 54.0, .time = 27.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 243.0,
        .damage = 216.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = { .clump_radius = 9, .poison_damage = { .damage = 162.0, .time = 81.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 729.0,
        .damage = 648.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = { .clump_radius = 9, .poison_damage = { .damage = 486.0, .time = 243.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 1458.0,
        .damage = 1296.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = { .clump_radius = 9, .poison_damage = { .damage = 972.0, .time = 486.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 2916.0,
        .damage = 2592.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = { .clump_radius = 9, .poison_damage = { .damage = 1944.0, .time = 972.0 } }
    },
    {
        .name = "Poison Spore",
        .description = "A purple spore that poisons on contact",
        .health = 5832.0,
        .damage = 5184.0,
        .radius = 12.0,
        .reload = 2.5,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = { .clump_radius = 9, .poison_damage = { .damage = 3888.0, .time = 1944.0 } }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 21.0,
        .damage = 24.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.5,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 63.0,
        .damage = 72.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.5,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 126.0,
        .damage = 144.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 252.0,
        .damage = 288.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.5,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen",
        .description = "Scatter pollen shots when defending",
        .health = 504.0,
        .damage = 576.0,
        .radius = 7.0,
        .reload = 1.5,
        .count = 3,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 126.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 9.0,
            .aura_radius = 285.0,
            .aura_duration = 4.0,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 378.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 27.0,
            .aura_radius = 855.0,
            .aura_duration = 4.0,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 756.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 54.0,
            .aura_radius = 1710.0,
            .aura_duration = 4.0,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 1512.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 108.0,
            .aura_radius = 3420.0,
            .aura_duration = 4.0,
            .defend_only = 1
        }
    },
    {
        .name = "Pollen Sandstorm",
        .description = "Healing aura. Garden x Desert border rite.",
        .health = 3024.0,
        .damage = 0.0,
        .radius = 18.0,
        .reload = 7.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 216.0,
            .aura_radius = 6840.0,
            .aura_duration = 4.0,
            .defend_only = 1
        }
    },
    {
        .name = "Queen's Emblem",
        .description = "A royal badge that periodically summons a soldier ant.",
        .health = 165.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kMythic,
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
        .health = 330.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUltra,
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
        .health = 660.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kSuper,
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
        .health = 1320.0,
        .damage = 0.0,
        .radius = 12.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 5.0,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kSoldierAnt,
            .spawn_count = 2
        }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 87.0,
        .damage = 24.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 261.0,
        .damage = 72.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 783.0,
        .damage = 216.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 1566.0,
        .damage = 432.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 3132.0,
        .damage = 864.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Quicksand Orb",
        .description = "Slows on contact. Sealed with a sandstorm eye.",
        .health = 6264.0,
        .damage = 1728.0,
        .radius = 14.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 0.65 }
    },
    {
        .name = "Rice",
        .description = "Spawns instantly, but not very strong",
        .health = 3.0,
        .damage = 12.0,
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
        .health = 9.0,
        .damage = 36.0,
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
        .health = 18.0,
        .damage = 72.0,
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
        .health = 36.0,
        .damage = 144.0,
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
        .health = 72.0,
        .damage = 288.0,
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
        .health = 111.0,
        .damage = 24.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .poison_damage = { .damage = 12.0, .time = 15.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 333.0,
        .damage = 72.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 36.0, .time = 45.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 999.0,
        .damage = 216.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 108.0, .time = 135.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 1998.0,
        .damage = 432.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 216.0, .time = 270.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 3996.0,
        .damage = 864.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 432.0, .time = 540.0 } }
    },
    {
        .name = "Rotwood",
        .description = "Lingering poison. Jungle rotwood craft.",
        .health = 7992.0,
        .damage = 1728.0,
        .radius = 16.0,
        .reload = 4.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 864.0, .time = 1080.0 } }
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
            .extra_rotation_speed = 0.35f
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
            .extra_rotation_speed = 0.35f
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
            .extra_rotation_speed = 0.35f
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
            .extra_rotation_speed = 0.35f
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
            .extra_rotation_speed = 0.35f
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
            .extra_rotation_speed = 0.35f
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
            .extra_rotation_speed = 0.35f
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
        .attributes = { .armor = 10, .extra_health = 25 }
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
        .attributes = { .armor = 10, .extra_health = 25 }
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
        .attributes = { .armor = 10, .extra_health = 25 }
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
        .attributes = { .armor = 10, .extra_health = 25 }
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
        .attributes = { .armor = 10, .extra_health = 25 }
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
            .poison_damage = { .damage = 24.0, .time = 9.0 }
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
            .poison_damage = { .damage = 72.0, .time = 27.0 }
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
            .poison_damage = { .damage = 216.0, .time = 81.0 }
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
            .poison_damage = { .damage = 432.0, .time = 162.0 }
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
            .poison_damage = { .damage = 864.0, .time = 324.0 }
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
            .poison_damage = { .damage = 1728.0, .time = 648.0 }
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
        .health = 108.0,
        .damage = 99.0,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .extra_damage_factor = 1.16f,
            .execute_threshold = 0.25f,
            .execute_factor = 1.6f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 324.0,
        .damage = 297.0,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .extra_damage_factor = 1.16f,
            .execute_threshold = 0.25f,
            .execute_factor = 1.6f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 648.0,
        .damage = 594.0,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_damage_factor = 1.16f,
            .execute_threshold = 0.25f,
            .execute_factor = 1.6f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 1296.0,
        .damage = 1188.0,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_damage_factor = 1.16f,
            .execute_threshold = 0.25f,
            .execute_factor = 1.6f
        }
    },
    {
        .name = "Royal Thorn",
        .description = "A royal thorn that executes low-health foes with amplified damage",
        .health = 2592.0,
        .damage = 2376.0,
        .radius = 14.0,
        .reload = 4.4,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_damage_factor = 1.16f,
            .execute_threshold = 0.25f,
            .execute_factor = 1.6f
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
        .damage = 30.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .damage_reflection = 0.25
        }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 90.0,
        .damage = 90.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .damage_reflection = 0.25
        }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 270.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .damage_reflection = 0.25
        }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 540.0,
        .damage = 540.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .damage_reflection = 0.25
        }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 1080.0,
        .damage = 1080.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .damage_reflection = 0.25
        }
    },
    {
        .name = "Salt",
        .description = "Reflects some damage dealt to the flower. Does not stack with itself",
        .health = 2160.0,
        .damage = 2160.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .damage_reflection = 0.25
        }
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
        .health = 90.0,
        .damage = 66.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.0,
            .defend_only = 1,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 270.0,
        .damage = 198.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 2.0,
            .defend_only = 1,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 540.0,
        .damage = 396.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 2.0,
            .defend_only = 1,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 1080.0,
        .damage = 792.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 2.0,
            .defend_only = 1,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Leaf Dart",
        .description = "Launches a projectile at regular intervals.",
        .health = 2160.0,
        .damage = 1584.0,
        .radius = 10.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 2.0,
            .defend_only = 1,
            .icon_angle = 1,
            .rotation_style = PetalAttributes::kFollowRot
        }
    },
    {
        .name = "Sand Spike",
        .description = "Brittle hard spike. Desert traveler's dart.",
        .health = 42.0,
        .damage = 69.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Sand Spike",
        .description = "Brittle hard spike. Desert traveler's dart.",
        .health = 126.0,
        .damage = 207.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Sand Spike",
        .description = "Brittle hard spike. Desert traveler's dart.",
        .health = 378.0,
        .damage = 621.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Sand Spike",
        .description = "Brittle hard spike. Desert traveler's dart.",
        .health = 1134.0,
        .damage = 1863.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Sand Spike",
        .description = "Brittle hard spike. Desert traveler's dart.",
        .health = 2268.0,
        .damage = 3726.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Sand Spike",
        .description = "Brittle hard spike. Desert traveler's dart.",
        .health = 4536.0,
        .damage = 7452.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Sand Spike",
        .description = "Brittle hard spike. Desert traveler's dart.",
        .health = 9072.0,
        .damage = 14904.0,
        .radius = 8.0,
        .reload = 2.7,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 210.0,
        .damage = 90.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .damage_reflection = 0.10,
            .burn_damage = { .damage = 5.0, .time = 3.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 630.0,
        .damage = 270.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .damage_reflection = 0.10,
            .burn_damage = { .damage = 5.0, .time = 3.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 1260.0,
        .damage = 540.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .damage_reflection = 0.10,
            .burn_damage = { .damage = 5.0, .time = 3.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 2520.0,
        .damage = 1080.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .damage_reflection = 0.10,
            .burn_damage = { .damage = 5.0, .time = 3.0 }
        }
    },
    {
        .name = "Scorch Palm",
        .description = "A scorching palm that burns and reflects some damage.",
        .health = 5040.0,
        .damage = 2160.0,
        .radius = 21.0,
        .reload = 5.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .damage_reflection = 0.10,
            .burn_damage = { .damage = 5.0, .time = 3.0 }
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
        .attributes = { .armor = 4 }
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
        .attributes = { .armor = 4 }
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
        .attributes = { .armor = 4 }
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
        .attributes = { .armor = 4 }
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
        .attributes = { .armor = 4 }
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
        .attributes = { .armor = 4 }
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
        .attributes = { .armor = 4 }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 204.0,
        .damage = 189.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_damage_factor = 1.20f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 408.0,
        .damage = 378.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_damage_factor = 1.20f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 816.0,
        .damage = 756.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_damage_factor = 1.20f }
    },
    {
        .name = "Skyblade",
        .description = "Amplifies petal damage. Relic sealed with twin Royal Seals.",
        .health = 1632.0,
        .damage = 1512.0,
        .radius = 18.0,
        .reload = 8.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_damage_factor = 1.20f }
    },
    {
        .name = "Snowball",
        .description = "Fast-returning snowballs. Snow local forge.",
        .health = 54.0,
        .damage = 24.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kRare,
        .attributes = {}
    },
    {
        .name = "Snowball",
        .description = "Fast-returning snowballs. Snow local forge.",
        .health = 162.0,
        .damage = 72.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kEpic,
        .attributes = {}
    },
    {
        .name = "Snowball",
        .description = "Fast-returning snowballs. Snow local forge.",
        .health = 486.0,
        .damage = 216.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Snowball",
        .description = "Fast-returning snowballs. Snow local forge.",
        .health = 1458.0,
        .damage = 648.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Snowball",
        .description = "Fast-returning snowballs. Snow local forge.",
        .health = 2916.0,
        .damage = 1296.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Snowball",
        .description = "Fast-returning snowballs. Snow local forge.",
        .health = 5832.0,
        .damage = 2592.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Snowball",
        .description = "Fast-returning snowballs. Snow local forge.",
        .health = 11664.0,
        .damage = 5184.0,
        .radius = 16.0,
        .reload = 1.1,
        .count = 2,
        .rarity = RarityID::kUnique,
        .attributes = {}
    },
    {
        .name = "Soldier Egg",
        .description = "Hatches a soldier ant. Colony egg rite.",
        .health = 144.0,
        .damage = 0.0,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
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
        .health = 432.0,
        .damage = 0.0,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kMythic,
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
        .health = 864.0,
        .damage = 0.0,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUltra,
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
        .health = 1728.0,
        .damage = 0.0,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kSuper,
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
        .health = 3456.0,
        .damage = 0.0,
        .radius = 17.0,
        .reload = 12.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 2.5,
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
        .health = 24.0,
        .damage = 42.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .poison_damage = { .damage = 18.0, .time = 9.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 72.0,
        .damage = 126.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .poison_damage = { .damage = 54.0, .time = 27.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 144.0,
        .damage = 252.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .poison_damage = { .damage = 108.0, .time = 54.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 288.0,
        .damage = 504.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .poison_damage = { .damage = 216.0, .time = 108.0 } }
    },
    {
        .name = "Spore Cloud",
        .description = "A drifting cloud of poisonous spores",
        .health = 576.0,
        .damage = 1008.0,
        .radius = 16.0,
        .reload = 4.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .poison_damage = { .damage = 432.0, .time = 216.0 } }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 102.0,
        .damage = 51.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.75f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 306.0,
        .damage = 153.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.75f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 918.0,
        .damage = 459.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.75f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 1836.0,
        .damage = 918.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.75f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 3672.0,
        .damage = 1836.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.75f }
    },
    {
        .name = "Starfish",
        .description = "Third hit hits harder. Reef local forge.",
        .health = 7344.0,
        .damage = 3672.0,
        .radius = 15.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .hit_cycle = 3, .hit_cycle_factor = 1.75f }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 114.0,
        .damage = 75.0,
        .radius = 12.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 6 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 342.0,
        .damage = 225.0,
        .radius = 12.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 6 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 1026.0,
        .damage = 675.0,
        .radius = 12.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 6 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 2052.0,
        .damage = 1350.0,
        .radius = 12.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 6 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 4104.0,
        .damage = 2700.0,
        .radius = 12.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 6 }
    },
    {
        .name = "Steel Sword",
        .description = "Solid steel. Capital local forge.",
        .health = 8208.0,
        .damage = 5400.0,
        .radius = 12.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 6 }
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
        .health = 60.0,
        .damage = 6.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUltra,
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
        .health = 120.0,
        .damage = 12.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kSuper,
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
        .health = 240.0,
        .damage = 24.0,
        .radius = 15.0,
        .reload = 3.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { 
            .secondary_reload = 4.0,
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
            .armor = 10
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
            .armor = 10
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
            .armor = 10
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
            .armor = 10
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
            .armor = 10
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
            .armor = 10
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
            .armor = 10
        }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 246.0,
        .damage = 18.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 5, .poison_armor = 18 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 738.0,
        .damage = 54.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 5, .poison_armor = 18 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 1476.0,
        .damage = 108.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 5, .poison_armor = 18 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 2952.0,
        .damage = 216.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 5, .poison_armor = 18 }
    },
    {
        .name = "Symbiote Shell",
        .description = "A protective shell with armor and strong poison resistance",
        .health = 5904.0,
        .damage = 432.0,
        .radius = 22.0,
        .reload = 6.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 5, .poison_armor = 18 }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 80.0,
        .damage = 52.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .extra_range = 75,
            .equipment = EquipmentFlags::kThirdEye
        }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 160.0,
        .damage = 104.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .extra_range = 75,
            .equipment = EquipmentFlags::kThirdEye
        }
    },
    {
        .name = "Third Eye",
        .description = "Allows your flower to extend petals further out",
        .health = 320.0,
        .damage = 208.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .extra_range = 75,
            .equipment = EquipmentFlags::kThirdEye
        }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 234.0,
        .damage = 60.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .damage_reflection = 0.18 }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 702.0,
        .damage = 180.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .damage_reflection = 0.18 }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 1404.0,
        .damage = 360.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .damage_reflection = 0.18 }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 2808.0,
        .damage = 720.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .damage_reflection = 0.18 }
    },
    {
        .name = "Thorn Palm",
        .description = "Reflects damage. Garden x Desert border rite.",
        .health = 5616.0,
        .damage = 1440.0,
        .radius = 21.0,
        .reload = 5.2,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .damage_reflection = 0.18 }
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
            .slow_inflict_seconds = 1.5f,
            .poison_damage = { .damage = 12.0, .time = 6.0 }
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
            .slow_inflict_seconds = 1.5f,
            .poison_damage = { .damage = 36.0, .time = 18.0 }
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
            .slow_inflict_seconds = 1.5f,
            .poison_damage = { .damage = 108.0, .time = 54.0 }
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
            .slow_inflict_seconds = 1.5f,
            .poison_damage = { .damage = 216.0, .time = 108.0 }
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
            .slow_inflict_seconds = 1.5f,
            .poison_damage = { .damage = 432.0, .time = 216.0 }
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
            .slow_inflict_seconds = 1.5f,
            .poison_damage = { .damage = 864.0, .time = 432.0 }
        }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 87.0,
        .damage = 21.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kRare,
        .attributes = { .slow_inflict_seconds = 1.1, .snare_seconds = 0.8 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 261.0,
        .damage = 63.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .slow_inflict_seconds = 1.1, .snare_seconds = 0.8 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 783.0,
        .damage = 189.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .slow_inflict_seconds = 1.1, .snare_seconds = 0.8 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 2349.0,
        .damage = 567.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .slow_inflict_seconds = 1.1, .snare_seconds = 0.8 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 4698.0,
        .damage = 1134.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .slow_inflict_seconds = 1.1, .snare_seconds = 0.8 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 9396.0,
        .damage = 2268.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .slow_inflict_seconds = 1.1, .snare_seconds = 0.8 }
    },
    {
        .name = "Thorn Vine",
        .description = "Snares and slows. Jungle local forge.",
        .health = 18792.0,
        .damage = 4536.0,
        .radius = 15.0,
        .reload = 3.6,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .slow_inflict_seconds = 1.1, .snare_seconds = 0.8 }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 90.0,
        .damage = 36.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = {
            .secondary_reload = 2.5,
            .extra_range = 18,
            .radial_pulse_damage = 14.0,
            .radial_pulse_radius = 90.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 270.0,
        .damage = 108.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {
            .secondary_reload = 2.5,
            .extra_range = 18,
            .radial_pulse_damage = 14.0,
            .radial_pulse_radius = 90.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 810.0,
        .damage = 324.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 2.5,
            .extra_range = 18,
            .radial_pulse_damage = 14.0,
            .radial_pulse_radius = 90.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 1620.0,
        .damage = 648.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 2.5,
            .extra_range = 18,
            .radial_pulse_damage = 14.0,
            .radial_pulse_radius = 90.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 3240.0,
        .damage = 1296.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 2.5,
            .extra_range = 18,
            .radial_pulse_damage = 14.0,
            .radial_pulse_radius = 90.0
        }
    },
    {
        .name = "Tide Pearl",
        .description = "Orbit + water ring. Ocean tide craft.",
        .health = 6480.0,
        .damage = 2592.0,
        .radius = 13.0,
        .reload = 3.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 2.5,
            .extra_range = 18,
            .radial_pulse_damage = 14.0,
            .radial_pulse_radius = 90.0
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
        .attributes = { .armor = 8 }
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
        .attributes = { .armor = 8 }
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
        .attributes = { .armor = 8 }
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
        .attributes = { .armor = 8 }
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
        .attributes = { .armor = 8 }
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
        .attributes = { .armor = 8 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 330.0,
        .damage = 24.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .armor = 16, .extra_health = 18, .taunt_radius = 200.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 990.0,
        .damage = 72.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .armor = 16, .extra_health = 18, .taunt_radius = 200.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 1980.0,
        .damage = 144.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .armor = 16, .extra_health = 18, .taunt_radius = 200.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 3960.0,
        .damage = 288.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .armor = 16, .extra_health = 18, .taunt_radius = 200.0 }
    },
    {
        .name = "Treant Guard",
        .description = "A heavily armored woodguard that taunts nearby foes.",
        .health = 7920.0,
        .damage = 576.0,
        .radius = 24.0,
        .reload = 8.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .armor = 16, .extra_health = 18, .taunt_radius = 200.0 }
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 15.0,
        .damage = 24.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kLegendary,
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
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 90.0,
        .damage = 144.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 180.0,
        .damage = 288.0,
        .radius = 7.0,
        .reload = 1.0,
        .count = 3,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Triplet",
        .description = "Three light petals in one slot",
        .health = 360.0,
        .damage = 576.0,
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
        .attributes = { .mass = 80.0, .armor = 3 }
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
        .attributes = { .mass = 80.0, .armor = 3 }
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
        .attributes = { .mass = 80.0, .armor = 3 }
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
        .attributes = { .mass = 80.0, .armor = 3 }
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
        .attributes = { .mass = 80.0, .armor = 3 }
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
        .attributes = { .mass = 80.0, .armor = 3 }
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
        .attributes = { .armor = 4, .poison_damage = { .damage = 27.0, .time = 13.5 } }
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
        .attributes = { .armor = 4, .poison_damage = { .damage = 81.0, .time = 40.5 } }
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
        .attributes = { .armor = 4, .poison_damage = { .damage = 162.0, .time = 81.0 } }
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
        .attributes = { .armor = 4, .poison_damage = { .damage = 324.0, .time = 162.0 } }
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
        .attributes = { .armor = 4, .poison_damage = { .damage = 648.0, .time = 324.0 } }
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
        .health = 195.0,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
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
        .health = 585.0,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kMythic,
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
        .health = 1170.0,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kUltra,
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
        .health = 2340.0,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kSuper,
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
        .health = 4680.0,
        .damage = 0.0,
        .radius = 21.0,
        .reload = 16.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 3.5,
            .defend_only = 1,
            .rotation_style = PetalAttributes::kNoRot,
            .spawns = MobID::kMagmaCentipede
        }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 120.0,
        .damage = 9.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kEpic,
        .attributes = { .extra_health = 8, .splash_damage = 8.0, .splash_radius = 55.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 360.0,
        .damage = 27.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .extra_health = 8, .splash_damage = 8.0, .splash_radius = 55.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 1080.0,
        .damage = 81.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .extra_health = 8, .splash_damage = 8.0, .splash_radius = 55.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 2160.0,
        .damage = 162.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .extra_health = 8, .splash_damage = 8.0, .splash_radius = 55.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 4320.0,
        .damage = 324.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .extra_health = 8, .splash_damage = 8.0, .splash_radius = 55.0, .defend_only = 1 }
    },
    {
        .name = "Water Bubble",
        .description = "Absorbs and splashes. Ocean local forge.",
        .health = 8640.0,
        .damage = 648.0,
        .radius = 18.0,
        .reload = 2.8,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .extra_health = 8, .splash_damage = 8.0, .splash_radius = 55.0, .defend_only = 1 }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 66.0,
        .damage = 39.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = { .move_speed_factor = 1.11f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 198.0,
        .damage = 117.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = { .move_speed_factor = 1.11f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 396.0,
        .damage = 234.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = { .move_speed_factor = 1.11f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 792.0,
        .damage = 468.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = { .move_speed_factor = 1.11f }
    },
    {
        .name = "Wet Leaf",
        .description = "A light leaf that improves movement speed.",
        .health = 1584.0,
        .damage = 936.0,
        .radius = 14.0,
        .reload = 2.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = { .move_speed_factor = 1.11f }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 255.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 21.0,
            .aura_radius = 465.0,
            .aura_duration = 8.0,
            .defend_only = 1
        }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 510.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 42.0,
            .aura_radius = 930.0,
            .aura_duration = 8.0,
            .defend_only = 1
        }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 1020.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 84.0,
            .aura_radius = 1860.0,
            .aura_duration = 8.0,
            .defend_only = 1
        }
    },
    {
        .name = "World Tree Seed",
        .description = "Large heal aura. World-tree relic grown from Heart of Nature.",
        .health = 2040.0,
        .damage = 0.0,
        .radius = 22.0,
        .reload = 13.0,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {
            .secondary_reload = 0.5,
            .aura_heal = 168.0,
            .aura_radius = 3720.0,
            .aura_duration = 8.0,
            .defend_only = 1
        }
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 45.0,
        .damage = 45.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kLegendary,
        .attributes = {}
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 135.0,
        .damage = 135.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kMythic,
        .attributes = {}
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 270.0,
        .damage = 270.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUltra,
        .attributes = {}
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 540.0,
        .damage = 540.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kSuper,
        .attributes = {}
    },
    {
        .name = "Yin Yang",
        .description = "Alters the flower's petal rotation in interesting ways",
        .health = 1080.0,
        .damage = 1080.0,
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .rarity = RarityID::kUnique,
        .attributes = {}
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
        .drops = { PetalID::kStoneHeart, PetalID::kIronGuard, PetalID::kMatSteel },
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
        .drops = { PetalID::kRock, PetalID::kHeavy },
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
            PetalID::kMatIceScale, PetalID::kMatColdIron
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
            PetalID::kRock, PetalID::kHeavy, PetalID::kSalt, PetalID::kMatSulfur
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
            PetalID::kMatObsidian
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
        .drops = { PetalID::kRock, PetalID::kHeavy },
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
        .drops = { PetalID::kRock, PetalID::kHeavy },
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

// Relative rarity weights for multi-drop rolls.
// Per-mob: only rarities present in that mob's drop table are kept, then renormalized to 1.0.
// 2026-10 economy Seg2: shift mass toward Epic–Mythic; Ultra+ still rare (~×2 from prior).
static float const DROP_RARITY_WEIGHT[RarityID::kNumRarities] = {
    48.0f,  // Common
    26.0f,  // Unusual
    13.0f,  // Rare
     5.5f,  // Epic (~×1.22)
     2.0f,  // Legendary (~×1.25)
     0.60f, // Mythic (~×1.5)
     0.02f, // Ultra (~×2)
     0.04f, // Super (~×2)
     0.02f  // Unique (~×2; still far below Mythic)
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
