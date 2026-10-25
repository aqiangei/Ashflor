#pragma once

#include <Shared/StaticDefinitions.hh>

#include <cstdint>

typedef uint16_t game_tick_t;

#define PERCOMPONENT \
    COMPONENT(Physics) \
    COMPONENT(Camera) \
    COMPONENT(Relations) \
    COMPONENT(Flower) \
    COMPONENT(Petal) \
    COMPONENT(Health) \
    COMPONENT(Mob) \
    COMPONENT(Drop) \
    COMPONENT(Segmented) \
    COMPONENT(Web) \
    COMPONENT(Score) \
    COMPONENT(Name)

#define PERFIELD \
FIELDS_Physics \
FIELDS_Camera \
FIELDS_Relations \
FIELDS_Flower \
FIELDS_Petal \
FIELDS_Health \
FIELDS_Mob \
FIELDS_Drop \
FIELDS_Segmented \
FIELDS_Score \
FIELDS_Name

#define FIELDS_Physics \
SINGLE(Physics, x, Float) \
SINGLE(Physics, y, Float) \
SINGLE(Physics, radius, Float) \
SINGLE(Physics, angle, Float)

#define FIELDS_Camera \
SINGLE(Camera, player, EntityID) \
SINGLE(Camera, respawn_level, uint32_t) \
MULTIPLE(Camera, inventory, PetalID::T, 2 * MAX_SLOT_COUNT) \
SINGLE(Camera, killed_by, std::string) \
SINGLE(Camera, camera_x, Float) \
SINGLE(Camera, camera_y, Float) \
SINGLE(Camera, fov, Float) 

#define FIELDS_Relations \
SINGLE(Relations, team, EntityID) \
SINGLE(Relations, parent, EntityID) \
SINGLE(Relations, color, uint8_t)

#define FIELDS_Flower \
SINGLE(Flower, loadout_count, uint8_t) \
SINGLE(Flower, face_flags, uint8_t) \
SINGLE(Flower, equip_flags, uint8_t) \
SINGLE(Flower, effect_flags, uint8_t) \
SINGLE(Flower, mana_ratio, Float) \
SINGLE(Flower, shield_ratio, Float) \
SINGLE(Flower, ability_cooldown, uint16_t) \
MULTIPLE(Flower, loadout_ids, PetalID::T, 2 * MAX_SLOT_COUNT) \
MULTIPLE(Flower, loadout_reloads, uint8_t, MAX_SLOT_COUNT) \
MULTIPLE(Flower, loadout_healths, uint8_t, MAX_SLOT_COUNT)

#define FIELDS_Petal \
SINGLE(Petal, petal_id, PetalID::T) \
SINGLE(Petal, split_projectile, uint8_t)

#define FIELDS_Health \
SINGLE(Health, health_ratio, Float) \
SINGLE(Health, hit_damage, Float) \
SINGLE(Health, damaged, StickyFlag)

#define FIELDS_Mob \
SINGLE(Mob, mob_id, MobID::T) \
SINGLE(Mob, rarity, uint8_t)

#define FIELDS_Drop \
SINGLE(Drop, drop_id, PetalID::T) \
SINGLE(Drop, drop_count, uint16_t)

#define FIELDS_Segmented

#define FIELDS_Web

#define FIELDS_Score \
SINGLE(Score, score, uint32_t)

#define FIELDS_Name \
SINGLE(Name, name, std::string) \
SINGLE(Name, nametag_visible, uint8_t) \
SINGLE(Name, guild, std::string)

#ifdef SERVERSIDE
#define PER_EXTRA_FIELD \
    SINGLE(velocity, Vector, .set(0,0)) \
    SINGLE(collision_velocity, Vector, .set(0,0)) \
    SINGLE(acceleration, Vector, .set(0,0)) \
    SINGLE(friction, float, =0) \
    SINGLE(mass, float, =1) \
    SINGLE(speed_ratio, float, =1) \
    \
    MULTIPLE(loadout, LoadoutSlot, MAX_SLOT_COUNT, .reset()) \
    SINGLE(heading_angle, float, =0) \
    SINGLE(player_count, uint32_t, =0) \
    SINGLE(flags, uint16_t, =0) \
    SINGLE(input, uint8_t, =0) \
    \
    SINGLE(slow_ticks, game_tick_t, =0) \
    SINGLE(slow_inflict, game_tick_t, =0) \
    SINGLE(immunity_ticks, game_tick_t, =0) \
    SINGLE(dandy_ticks, game_tick_t, =0) \
    SINGLE(poison_ticks, game_tick_t, =0) \
    SINGLE(burn_ticks, game_tick_t, =0) \
    SINGLE(snare_ticks, game_tick_t, =0) \
    SINGLE(corruption_ticks, game_tick_t, =0) \
    SINGLE(despawn_tick, game_tick_t, =0) \
    SINGLE(secondary_reload, game_tick_t, =0) \
    SINGLE(ai_tick, game_tick_t, =0) \
    SINGLE(contact_hit_count, uint8_t, =0) \
    /* Wiki-import flower state (server-only; not networked). */ \
    SINGLE(mana, float, =0) \
    SINGLE(flower_shield, float, =0) \
    SINGLE(totem_heal_rate, float, =0) \
    SINGLE(totem_ticks, game_tick_t, =0) \
    SINGLE(root_stacks, uint8_t, =0) \
    SINGLE(root_tick, game_tick_t, =0) \
    SINGLE(ankh_x, float, =0) \
    SINGLE(ankh_y, float, =0) \
    SINGLE(ankh_set, uint8_t, =0) \
    SINGLE(burrow_ticks, game_tick_t, =0) \
    SINGLE(wax_charges, uint8_t, =0) \
    SINGLE(death_delay_ticks, game_tick_t, =0) \
    SINGLE(death_delay_used, uint8_t, =0) \
    SINGLE(previous_input, uint8_t, =0) \
    SINGLE(burrow_cooldown, game_tick_t, =0) \
    SINGLE(root_armor_applied, float, =0) \
    SINGLE(armor_shredded, float, =0) \
    SINGLE(armor_shred_ticks, game_tick_t, =0) \
    SINGLE(powder_suppressed_ticks, game_tick_t, =0) \
    SINGLE(pending_damage, float, =0) \
    SINGLE(pending_damage_dealer, EntityID, =NULL_ENTITY) \
    SINGLE(pending_damage_ticks, game_tick_t, =0) \
    SINGLE(dark_marked_ticks, game_tick_t, =0) \
    SINGLE(champion_aura, float, =0) \
    SINGLE(compass_reveal, uint8_t, =0) \
    SINGLE(soul_split_cd, game_tick_t, =0) \
    SINGLE(paranormal_vision, uint8_t, =0) \
    SINGLE(overheal_shield_ratio, float, =0) \
    \
    SINGLE(poison_inflicted, float, =0) \
    SINGLE(poison_dealer, EntityID, =NULL_ENTITY) \
    SINGLE(poison_damage, PoisonDamage, ={}) \
    SINGLE(burn_inflicted, float, =0) \
    SINGLE(burn_dealer, EntityID, =NULL_ENTITY) \
    SINGLE(burn_damage, PoisonDamage, ={}) \
    SINGLE(health, double, =0) \
    SINGLE(max_health, double, =0) \
    SINGLE(damage, float, =0) \
    SINGLE(armor, float, =0) \
    SINGLE(poison_armor, float, =0) \
    SINGLE(damage_reflection, float, =0) \
    SINGLE(last_damaged_by, EntityID, =NULL_ENTITY) \
    /* Per-player camera ids + cumulative damage for private loot eligibility.
       Damage persists for the mob's life (player respawn does not clear it). */ \
    SINGLE(loot_claim_n, uint8_t, =0) \
    MULTIPLE(loot_claim_teams, EntityID, 16, =NULL_ENTITY) \
    MULTIPLE(loot_claim_damage, float, 16, =0) \
    SINGLE(score_reward, uint32_t, =0) \
    \
    SINGLE(base_entity, EntityID, =NULL_ENTITY) \
    SINGLE(target, EntityID, =NULL_ENTITY) \
    SINGLE(seg_head, EntityID, =NULL_ENTITY) \
    SINGLE(detection_radius, float, =0) \
    SINGLE(ai_state, uint8_t, =0) \
    \
    SINGLE(zone, uint8_t, =0) \
    /* Wild leash home = spawn point (not zone AABB center — outer zones span the whole map) */ \
    SINGLE(home_x, float, =0) \
    SINGLE(home_y, float, =0) \
    /* florr unseenDespawn: ticks since last camera AOI; zone mobs cull after ~30s */ \
    SINGLE(unseen_ticks, game_tick_t, =0) \
    SINGLE(deletion_tick, uint8_t, =0) \
    SINGLE(deleted_petals, circ_arr_t, ={})
#else
#define PER_EXTRA_FIELD \
    SINGLE(last_damaged_time, double, =0) \
    SINGLE(healthbar_lag, float, =0) \
    SINGLE(healthbar_opacity, float, =0) \
    SINGLE(deletion_animation, float, =0) \
    SINGLE(eye_x, float, =3) \
    SINGLE(eye_y, float, =0) \
    SINGLE(mouth, float, =15) \
    SINGLE(animation, float, =0) \
    SINGLE(damage_flash, float, =0) \
    /* Floating combat text: last synced health_ratio.anchor() */ \
    SINGLE(prev_health_ratio, float, =1) \
    SINGLE(health_ratio_seen, uint8_t, =0)
#endif

class EntityID {
public:
    typedef uint8_t hash_type;
    typedef uint16_t id_type;
    id_type id;
    hash_type hash;
    EntityID();
    EntityID(id_type, hash_type);
    static uint32_t make_hash(EntityID const);
    static bool equal_to(EntityID const, EntityID const);
    bool null() const;
};

namespace std {
    template<>
    struct hash<EntityID> {
        size_t operator()(EntityID const &id) const {
            return (size_t) EntityID::make_hash(id);
        }
    };
}

bool operator<(EntityID const, EntityID const);
bool operator==(EntityID const, EntityID const);

inline const EntityID NULL_ENTITY;

#ifdef SERVER_ONLY
class Simulation;

struct LoadoutPetal {
    EntityID ent_id;
    game_tick_t reload;
};

class LoadoutSlot {
    PetalID::T id;
public:
    PetalID::T behavior_id = PetalID::kNone;
    uint8_t effect_charges = 0;
    game_tick_t effect_reload = 0;
    uint8_t already_spawned;
    LoadoutPetal petals[MAX_PETALS_IN_CLUMP];    
    LoadoutSlot();
    void reset();
    void update_id(Simulation *, PetalID::T);
    void force_reload();
    PetalID::T get_petal_id() const;
    uint32_t size() const;
};
#endif
