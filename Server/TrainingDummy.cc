#include <Server/TrainingDummy.hh>

#include <Server/Client.hh>
#include <Server/EntityFunctions.hh>
#include <Server/Game.hh>
#include <Server/Loot.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>
#include <Shared/Simulation.hh>
#include <Shared/StaticData.hh>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {
uint32_t const kResetTicks = 3 * TPS;

// Compact readout aligned with Helpers::format_score (lowercase k/m/b).
// <1k keeps one decimal so small DPS still reads; 1e6+ fits the HP label.
void format_stat(char *buf, size_t n, double value) {
    if (!std::isfinite(value) || value == 0.0) {
        std::snprintf(buf, n, "0.0");
        return;
    }
    double const a = std::fabs(value);
    if (a < 1000.0) {
        std::snprintf(buf, n, "%.1f", value);
        return;
    }
    char const *suf = "k";
    double scaled = value / 1000.0;
    if (a >= 1e9) {
        suf = "b";
        scaled = value / 1e9;
    } else if (a >= 1e6) {
        suf = "m";
        scaled = value / 1e6;
    }
    double const sa = std::fabs(scaled);
    // Fewer decimals as magnitude grows: 1.25k / 12.5k / 125k.
    if (sa >= 100.0)
        std::snprintf(buf, n, "%.0f%s", scaled, suf);
    else if (sa >= 10.0)
        std::snprintf(buf, n, "%.1f%s", scaled, suf);
    else
        std::snprintf(buf, n, "%.2f%s", scaled, suf);
}

uint64_t equipment_signature(Client *client) {
    if (!client->game) return 0;
    auto &sim = client->game->simulation;
    if (!sim.ent_alive(client->camera)) return 0;
    auto const &camera = sim.get_ent(client->camera);
    bool alive = sim.ent_alive(camera.get_player());
    uint64_t hash = 14695981039346656037ull;
    auto add = [&](uint32_t value) { hash = (hash ^ value) * 1099511628211ull; };
    add(sim.map_id);
    for (uint8_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i) {
        PetalID::T id = alive ? sim.get_ent(camera.get_player()).get_loadout_ids(i) : camera.get_inventory(i);
        add(id);
        if (id < PetalID::kNumPetals) add(client->petal_levels[id]);
    }
    for (uint8_t rank : client->talent_ranks) add(rank);
    add(client->rot_speed_setting);
    return hash;
}

void send_update(Simulation *sim, Client *client) {
    Writer writer(Server::OUTGOING_PACKET);
    writer.write<uint8_t>(Clientbound::kTrainingStats);
    for (auto const &stats : client->training_stats) {
        double seconds = std::max(1.0, (double)(sim->tick_index - stats.first_tick + 1) / TPS);
        char text[64];
        for (double value : { (double)stats.last_hit, stats.hit ? stats.damage / seconds : 0.0, stats.damage }) {
            format_stat(text, sizeof(text), value);
            writer.write<std::string>(text);
        }
    }
    // Deliberately unicast. Neither entity deltas nor party broadcasts carry damage stats.
    client->send_packet(writer.packet, writer.at - writer.packet);
    client->training_dirty = false;
}

void prepare(Client *client) {
    uint64_t signature = equipment_signature(client);
    if (client->training_equipment != signature) {
        client->training_stats = {};
        client->training_equipment = signature;
        client->training_dirty = true;
    }
}
}

float TrainingDummy::petal_contact_damage(Entity const &dummy) {
    uint8_t r = dummy.get_rarity();
    if (r >= RarityID::kNumRarities) r = 0;
    // ~basic mob body at Common; higher dummy tiers chip petals harder.
    return 10.f * mob_spawn_damage_mult(RarityID::kCommon, r);
}

void TrainingDummy::ensure(Simulation *sim) {
    if (sim->map_id != MapID::kGarden) return;
    for (uint8_t rarity = 0; rarity < RarityID::kNumRarities; ++rarity) {
        EntityID &id = sim->training_dummies[rarity];
        if (sim->ent_alive(id)) continue;
        float angle = -(float)M_PI / 2 + rarity * (2.f * (float)M_PI / RarityID::kNumRarities);
        float x = MAP_SPAWN_X + std::cos(angle) * 960.f;
        float y = MAP_SPAWN_Y + std::sin(angle) * 720.f;
        float radius = 44.f * mob_rarity_radius_mult(rarity);
        Map::resolve_circle_terrain(sim->map_id, x, y, radius + 8.f);
        Entity &ent = alloc_mob(sim, MobID::kTrainingDummy, x, y, NULL_ENTITY);
        ent.set_rarity(rarity);
        ent.set_color(rarity);
        ent.set_angle(0);
        ent.set_radius(radius);
        ent.mass = (1 + radius / BASE_FLOWER_RADIUS) * 10000.f;
        ent.max_health *= mob_spawn_threat_mult(RarityID::kCommon, rarity);
        ent.health = ent.max_health;
        ent.home_x = x;
        ent.home_y = y;
        ent.zone = 0xFF;
        ent.detection_radius = 0;
        BitMath::set(ent.flags, EntityFlags::kNoDrops);
        id = ent.id;
    }
}

TrainingDummy::Stats *TrainingDummy::personal_stats(Simulation *sim, Entity const &ent, EntityID source) {
    if (sim->map_id != MapID::kGarden || ent.get_rarity() >= RarityID::kNumRarities) return nullptr;
    if (!(sim->training_dummies[ent.get_rarity()] == ent.id)) return nullptr;
    Client *client = Server::client_by_camera(sim, Loot::owner_camera(sim, source));
    if (!client) return nullptr;
    prepare(client);
    return &client->training_stats[ent.get_rarity()];
}

void TrainingDummy::record_damage(Simulation *sim, Entity &ent, EntityID source, float amount) {
    Stats *stats = personal_stats(sim, ent, source);
    if (!stats || !is_finite(amount) || amount <= 0) return;
    if (!stats->hit || sim->tick_index - stats->last_tick >= kResetTicks) {
        *stats = {};
        stats->first_tick = sim->tick_index;
        stats->hit = true;
    }
    stats->damage += amount;
    stats->last_hit = amount;
    stats->last_tick = sim->tick_index;
    Server::client_by_camera(sim, Loot::owner_camera(sim, source))->training_dirty = true;
}

void TrainingDummy::inflict_status(Stats &stats, Entity const &attacker, uint8_t type) {
    stats.slow_ticks = std::max(stats.slow_ticks, attacker.slow_inflict);
    if (type != DamageType::kContact) return;
    game_tick_t poison = (game_tick_t)(attacker.poison_damage.time * TPS);
    if (stats.poison_ticks < poison) {
        stats.poison_ticks = poison;
        stats.poison_per_tick = attacker.poison_damage.damage / TPS;
    }
    game_tick_t burn = (game_tick_t)(attacker.burn_damage.time * TPS);
    if (stats.burn_ticks < burn) {
        stats.burn_ticks = burn;
        stats.burn_per_tick = attacker.burn_damage.damage / TPS;
    }
}

void TrainingDummy::reset(Client *client) {
    if (!client || !client->game) return;
    client->training_stats = {};
    client->training_equipment = equipment_signature(client);
    client->training_dirty = true;
    send_update(&client->game->simulation, client);
}

void TrainingDummy::tick_client(Simulation *sim, Client *client) {
    prepare(client);
    bool active = false;
    for (uint8_t rarity = 0; rarity < RarityID::kNumRarities; ++rarity) {
        Stats &stats = client->training_stats[rarity];
        EntityID id = sim->training_dummies[rarity];
        if (sim->map_id == MapID::kGarden && sim->ent_alive(id)) {
            // DOTs are private too: one player's poison cannot replace a teammate's.
            if (stats.poison_ticks > 0) {
                --stats.poison_ticks;
                inflict_damage(sim, client->camera, id, stats.poison_per_tick, DamageType::kPoison);
            }
            if (stats.burn_ticks > 0) {
                --stats.burn_ticks;
                inflict_damage(sim, client->camera, id, stats.burn_per_tick, DamageType::kBurn);
            }
            if (stats.slow_ticks > 0) --stats.slow_ticks;
        }
        if (stats.hit && sim->tick_index - stats.last_tick >= kResetTicks) {
            stats = {};
            client->training_dirty = true;
        }
        active |= stats.hit;
    }
    if ((client->training_dirty || active) && sim->tick_index % std::max(1u, TPS / 5u) == 0)
        send_update(sim, client);
}
