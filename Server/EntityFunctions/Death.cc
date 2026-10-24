#include <Server/EntityFunctions.hh>

#include <Server/AccountStore.hh>
#include <Server/Chat.hh>
#include <Server/Client.hh>
#include <Server/Game.hh>
#include <Server/Party.hh>
#include <Server/PetalTracker.hh>
#include <Server/Server.hh>
#include <Server/Spawn.hh>
#include <Server/Treasure.hh>

#include <Shared/Craft.hh>
#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/RarityScale.hh>
#include <Shared/Recipe.hh>
#include <Shared/Simulation.hh>
#include <Shared/SpawnRules.hh>
#include <Shared/StaticData.hh>

#include <Helpers/Math.hh>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

static std::string announce_player_name(Client *client) {
    if (client == nullptr || client->game == nullptr) return "无名";
    Simulation *sim = &client->game->simulation;
    if (!sim->ent_exists(client->camera)) return "无名";
    Entity &camera = sim->get_ent(client->camera);
    if (sim->ent_alive(camera.get_player())) {
        std::string n = sim->get_ent(camera.get_player()).get_name();
        if (!n.empty()) return n;
    }
    if (!client->account_name.empty()) return client->account_name;
    if (!client->play_name.empty()) return client->play_name;
    return "无名";
}

static std::string announce_party_names(GameInstance *game, Client *mvp) {
    if (mvp == nullptr) return "无名";
    if (game == nullptr || mvp->party_id == 0)
        return announce_player_name(mvp);
    std::string out;
    for (Client *c : game->clients) {
        if (c == nullptr || c->party_id != mvp->party_id) continue;
        if (!out.empty()) out += "、";
        out += announce_player_name(c);
    }
    if (out.empty()) return announce_player_name(mvp);
    return out;
}

static void credit_boss_kill_stats(GameInstance *game, Client *mvp) {
    if (mvp == nullptr) return;
    if (game == nullptr || mvp->party_id == 0) {
        if (mvp->stat_kills < 4000000000u) ++mvp->stat_kills;
        return;
    }
    for (Client *c : game->clients) {
        if (c == nullptr || c->party_id != mvp->party_id) continue;
        if (c->stat_kills < 4000000000u) ++c->stat_kills;
    }
}

void flush_flower_loadout_to_camera(Simulation *sim, Entity &camera, Entity const &flower) {
    if (sim == nullptr || !flower.has_component(kFlower)) return;
    uint32_t const total_slots = flower.get_loadout_count() + MAX_SLOT_COUNT;
    uint32_t level = score_to_level(flower.get_score());
    if (level < 1) level = 1;
    if (level < camera.get_respawn_level())
        level = camera.get_respawn_level();
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        PetalTracker::remove_petal(sim, camera.get_inventory(i));
    camera.set_respawn_level(level);
    // Preserve mid-level XP for the next spawn (death / map transfer).
    if (Client *owner = Server::client_by_camera(sim, camera.id)) {
        uint32_t sc = flower.get_score();
        uint32_t floor = level_to_score(level);
        if (sc < floor) sc = floor;
        AccountStore::note_score(owner, sc);
    }
    for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        camera.set_inventory(i, PetalID::kNone);
    for (uint32_t i = 0; i < total_slots && i < 2 * MAX_SLOT_COUNT; ++i) {
        PetalID::T id = flower.get_loadout_ids(i);
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) continue;
        camera.set_inventory(i, id);
        PetalTracker::add_petal(sim, id);
    }
}

static void _alloc_drops(Simulation *sim, std::vector<PetalID::T> &success_drops, float x, float y, EntityID team) {
    #ifdef DEBUG
    for (PetalID::T id : success_drops)
        assert(id != PetalID::kNone && id < PetalID::kNumPetals);
    #endif
    size_t count = success_drops.size();
    for (size_t i = count; i > 0; --i) {
        PetalID::T drop_id = success_drops[i - 1];
        if (PETAL_DATA[drop_id].rarity == RarityID::kUnique && PetalTracker::get_count(sim, drop_id) > 0) {
            success_drops[i - 1] = success_drops[count - 1];
            --count;
            success_drops.pop_back();
        }
    }
    DEBUG_ONLY(assert(success_drops.size() == count);)
    // Coalesce identical ids into stacked piles (bags mats / multi-rolls).
    std::array<uint16_t, PetalID::kNumPetals> piles = {};
    std::vector<PetalID::T> order;
    order.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        PetalID::T id = success_drops[i];
        if (id == PetalID::kNone || id >= PetalID::kNumPetals) continue;
        if (piles[id] == 0) order.push_back(id);
        if (piles[id] < 65535) ++piles[id];
    }
    size_t n = order.size();
    for (size_t i = 0; i < n; ++i) {
        PetalID::T id = order[i];
        Entity &drop = alloc_drop(sim, id, team, piles[id]);
        float dx = x, dy = y;
        if (n > 1) {
            float ang = (float) (i * 2 * M_PI / n);
            dx += std::cos(ang) * 28.f;
            dy += std::sin(ang) * 28.f;
            drop.velocity.unit_normal(ang).set_magnitude(25);
        }
        Map::resolve_circle_terrain(sim->map_id, dx, dy, drop.get_radius() + 8.f);
        dx = fclamp(dx, drop.get_radius(), ARENA_WIDTH - drop.get_radius());
        dy = fclamp(dy, drop.get_radius(), ARENA_HEIGHT - drop.get_radius());
        drop.set_x(dx);
        drop.set_y(dy);
    }
}

// Drop rarity band: mob's own tier (20%), one below (60%), two below (20%).
// Floor at Common. Boss flag does not inflate rarity past the mob's tier.
static uint8_t _roll_drop_rarity(uint8_t mob_rarity, uint8_t /*boss*/) {
    if (mob_rarity >= RarityID::kNumRarities) mob_rarity = RarityID::kCommon;
    float r = frand();
    if (r < 0.20f) return mob_rarity;
    if (r < 0.80f) {
        if (mob_rarity > RarityID::kCommon) return (uint8_t)(mob_rarity - 1);
        return mob_rarity;
    }
    if (mob_rarity >= 2) return (uint8_t)(mob_rarity - 2);
    return RarityID::kCommon;
}

// Always pick from the mob's own drop table; ladder family seeds to the
// banded rarity. Full petal pool only as empty-table fallback (should be rare).
static PetalID::T _roll_mob_drop(Simulation *sim, MobID::T mob_id, uint8_t mob_rarity,
                                 uint8_t boss = 0) {
    struct MobData const &mob_data = MOB_DATA[mob_id];
    uint8_t drop_r = _roll_drop_rarity(mob_rarity, boss);

    if (mob_data.drops.size() == 0) {
        for (uint32_t attempt = 0; attempt < 8; ++attempt) {
            PetalID::T got = Craft::random_petal_of_rarity(drop_r);
            if (got == PetalID::kNone || got >= PetalID::kNumPetals) continue;
            if (PETAL_DATA[got].rarity != RarityID::kUnique
                || PetalTracker::get_count(sim, got) == 0)
                return got;
            drop_r = drop_r > RarityID::kCommon
                ? (uint8_t)(drop_r - 1) : RarityID::kCommon;
        }
        return Craft::random_petal_of_rarity(drop_r);
    }

    float weights[MAX_DROPS_PER_MOB] = {};
    float total_weight = Craft::mob_loot_weights(mob_id, mob_rarity, weights);
    if (total_weight <= 0.f) {
        PetalID::T fam = mob_data.drops[0];
        if (is_chase_loot_petal(fam)) return fam;
        PetalID::T resolved = Craft::petal_at_rarity(fam, drop_r);
        return resolved != PetalID::kNone ? resolved : fam;
    }
    for (uint32_t attempt = 0; attempt < 8; ++attempt) {
        float roll = frand() * total_weight;
        float acc = 0.f;
        PetalID::T chosen = mob_data.drops[0];
        for (uint32_t i = 0; i < mob_data.drops.size(); ++i) {
            acc += weights[i];
            chosen = mob_data.drops[i];
            if (roll < acc) break;
        }
        // Chase specials keep baked ID; everything else ladders to drop_r.
        PetalID::T out = chosen;
        if (!is_chase_loot_petal(chosen)) {
            PetalID::T resolved = Craft::petal_at_rarity(chosen, drop_r);
            if (resolved != PetalID::kNone) out = resolved;
        }
        if (PETAL_DATA[out].rarity != RarityID::kUnique
            || PetalTracker::get_count(sim, out) == 0)
            return out;
    }
    for (PetalID::T id : mob_data.drops) {
        if (is_chase_loot_petal(id)) {
            if (PETAL_DATA[id].rarity != RarityID::kUnique
                || PetalTracker::get_count(sim, id) == 0)
                return id;
            continue;
        }
        PetalID::T resolved = Craft::petal_at_rarity(id, drop_r);
        PetalID::T out = resolved != PetalID::kNone ? resolved : id;
        if (PETAL_DATA[out].rarity != RarityID::kUnique)
            return out;
    }
    return mob_data.drops[0];
}

// Mob XP only (no PvP score-steal). Alive party mates of the killer each get
// a full copy of the mob's XP.
static void _add_mob_xp(Simulation *sim, EntityID const killer_id, Entity const &mob) {
    if (!mob.has_component(kMob)) return;
    uint32_t xp = (uint32_t)(mob.score_reward * kill_xp_grant_mult() + 0.5f);
    if (xp == 0 && mob.score_reward > 0) xp = 1;
    if (xp == 0 || !sim->ent_exists(killer_id)) return;
    Entity &killer = sim->get_ent(killer_id);
    if (!killer.has_component(kScore)) return;

    if (killer.has_component(kFlower) && sim->ent_alive(killer.get_parent())) {
        Client *kc = Server::client_by_camera(sim, killer.get_parent());
        if (kc != nullptr && kc->party_id != 0 && kc->game != nullptr) {
            uint8_t any = 0;
            for (Client *c : kc->game->clients) {
                if (c == nullptr || c->party_id != kc->party_id) continue;
                if (!c->alive() || c->game == nullptr) continue;
                if (&c->game->simulation != sim) continue;
                if (!sim->ent_exists(c->camera)) continue;
                Entity &cam = sim->get_ent(c->camera);
                if (!sim->ent_alive(cam.get_player())) continue;
                Entity &flower = sim->get_ent(cam.get_player());
                if (!flower.has_component(kScore)) continue;
                uint32_t cur = flower.get_score();
                uint32_t cap = score_soft_cap();
                if (cur >= cap) continue;
                uint32_t next = cur + xp;
                if (next < cur || next > cap) next = cap;
                flower.set_score(next);
                if (Client *ow = Server::client_by_camera(sim, cam.id))
                    AccountStore::note_score(ow, next);
                any = 1;
            }
            if (any) return;
        }
    }
    {
        uint32_t cur = killer.get_score();
        uint32_t cap = score_soft_cap();
        if (cur < cap) {
            uint32_t next = cur + xp;
            if (next < cur || next > cap) next = cap;
            killer.set_score(next);
            if (Client *ow = Server::client_by_camera(sim, killer.get_parent()))
                AccountStore::note_score(ow, next);
        }
    }
}

void entity_on_death(Simulation *sim, Entity const &ent) {
    // Administrative cleanup must not count practice targets as mob kills.
    if (TrainingDummy::is_dummy(ent)) return;
    //don't do on_death for any despawned entity
    uint8_t natural_despawn = BitMath::at(ent.flags, EntityFlags::kIsDespawning) && ent.despawn_tick == 0;
    // XP from mobs only — never from flower death (old PvP score-steal removed).
    if (ent.has_component(kMob) && ent.score_reward > 0
        && sim->ent_exists(ent.last_damaged_by) && !natural_despawn) {
        EntityID killer_id = sim->get_ent(ent.last_damaged_by).base_entity;
        _add_mob_xp(sim, killer_id, ent);
    }
    if (ent.has_component(kFlower) && sim->ent_alive(ent.get_parent())) {
        Entity &camera = sim->get_ent(ent.get_parent());
        EntityID killer_id = sim->ent_exists(ent.last_damaged_by) ?
            sim->get_ent(ent.last_damaged_by).base_entity : NULL_ENTITY;
        if (sim->ent_alive(killer_id)) {
            Entity const &killer = sim->get_ent(killer_id);
            if (killer.has_component(kName)) camera.set_killed_by(killer.get_name());
            else camera.set_killed_by("");
            // No PvP kill counter — players cannot kill each other.
        } else if (ent.poison_ticks > 0) camera.set_killed_by("Poison");
        else if (ent.burn_ticks > 0) camera.set_killed_by("Burn");
        else camera.set_killed_by("");
    }
    if (ent.has_component(kMob)) {
        // Explore loot chests: kill-to-open rewards (not walk-up).
        if (!natural_despawn && ent.get_mob_id() == MobID::kLootChest)
            Treasure::on_loot_chest_killed(sim, ent);
        // Credit this-life mob kill to the damaging flower (heads only).
        if (!natural_despawn
            && !ent.has_component(kSegmented)
            && sim->ent_exists(ent.last_damaged_by)) {
            EntityID killer_id = sim->get_ent(ent.last_damaged_by).base_entity;
            if (sim->ent_alive(killer_id)) {
                Entity &killer = sim->get_ent(killer_id);
                if (killer.has_component(kFlower) && sim->ent_alive(killer.get_parent())) {
                    Client *owner = Server::client_by_camera(sim, killer.get_parent());
                    if (owner != nullptr)
                        client_credit_mob_kill(owner, ent.get_mob_id());
                }
            }
        }
        if (BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone))
            Map::remove_mob(sim, ent);
        if (!natural_despawn && !(BitMath::at(ent.flags, EntityFlags::kNoDrops))) {
            struct MobData const &mob_data = MOB_DATA[ent.get_mob_id()];
            uint8_t const live_rarity = ent.get_rarity() < RarityID::kNumRarities
                ? ent.get_rarity() : mob_data.rarity;

            // Private loot per camera. Party: if party damage sum ≥ threshold,
            // every connected mate on this map gets a roll tagged to their
            // camera, including downed mates who can collect after respawn.
            EntityID claim_cams[16];
            float claim_dmg[16];
            uint8_t claim_n = 0;
            {
                uint8_t raw_n = ent.loot_claim_n;
                if (raw_n > 16) raw_n = 16;
                for (uint8_t i = 0; i < raw_n; ++i) {
                    if (ent.loot_claim_teams[i].null()) continue;
                    claim_cams[claim_n] = ent.loot_claim_teams[i];
                    claim_dmg[claim_n] = ent.loot_claim_damage[i];
                    ++claim_n;
                }
            }
            GameInstance *game = Server::game_for(sim);
            auto client_for_cam = [&](EntityID cam) -> Client * {
                if (game == nullptr || cam.null()) return nullptr;
                Client *c = Server::client_by_camera(sim, cam);
                if (c != nullptr) return c;
                for (Client *x : game->clients) {
                    if (x == nullptr || x->game == nullptr) continue;
                    if (&x->game->simulation != sim) continue;
                    if (x->camera == cam) return x;
                }
                return nullptr;
            };
            float const max_hp = ent.max_health > 1.f ? ent.max_health : 1.f;
            uint8_t const boss_loot = spawn_class(ent.get_mob_id()) == SpawnClass::kBoss;
            float const need = max_hp * (boss_loot ? 0.01f : 0.05f);

            EntityID recipients[32];
            uint8_t recip_n = 0;
            auto add_recipient = [&](EntityID cam) {
                if (cam.null() || recip_n >= 32 || !sim->ent_alive(cam)
                    || !sim->get_ent(cam).has_component(kCamera)) return;
                for (uint8_t i = 0; i < recip_n; ++i)
                    if (recipients[i] == cam) return;
                recipients[recip_n++] = cam;
            };
            auto party_sum = [&](Client *a) -> float {
                float s = 0.f;
                if (a == nullptr) return s;
                for (uint8_t i = 0; i < claim_n; ++i) {
                    Client *b = client_for_cam(claim_cams[i]);
                    if (b == a || Party::same_party(a, b))
                        s += claim_dmg[i];
                }
                return s;
            };
            uint8_t party_done[16] = {};
            for (uint8_t i = 0; i < claim_n; ++i) {
                Client *c = client_for_cam(claim_cams[i]);
                float dmg = (c != nullptr && c->party_id != 0)
                    ? party_sum(c) : claim_dmg[i];
                if (dmg + 1e-3f < need) continue;
                if (c != nullptr && c->party_id != 0) {
                    uint8_t already = 0;
                    for (uint8_t p = 0; p < i; ++p) {
                        if (!party_done[p]) continue;
                        if (Party::same_party(c, client_for_cam(claim_cams[p]))) {
                            already = 1;
                            break;
                        }
                    }
                    if (already) continue;
                    party_done[i] = 1;
                    if (game != nullptr) {
                        for (Client *m : game->clients) {
                            if (m == nullptr || m->party_id != c->party_id) continue;
                            if (m->game == nullptr) continue;
                            if (&m->game->simulation != sim) continue;
                            if (!sim->ent_alive(m->camera)) continue;
                            add_recipient(m->camera);
                        }
                    } else {
                        add_recipient(claim_cams[i]);
                    }
                } else {
                    add_recipient(claim_cams[i]);
                }
            }

            // Boss announce: highest personal camera damage among claimers.
            if (spawn_class(ent.get_mob_id()) == SpawnClass::kBoss
                && !ent.has_component(kSegmented)) {
                Client *mvp = nullptr;
                if (claim_n > 0) {
                    uint8_t bi = 0;
                    for (uint8_t i = 1; i < claim_n; ++i) {
                        if (claim_dmg[i] > claim_dmg[bi]) bi = i;
                    }
                    mvp = client_for_cam(claim_cams[bi]);
                }
                if (mvp == nullptr && sim->ent_exists(ent.last_damaged_by)) {
                    EntityID kid = sim->get_ent(ent.last_damaged_by).base_entity;
                    if (sim->ent_alive(kid)) {
                        Entity &killer = sim->get_ent(kid);
                        if (killer.has_component(kFlower) && sim->ent_alive(killer.get_parent()))
                            mvp = Server::client_by_camera(sim, killer.get_parent());
                    }
                }
                if (mvp != nullptr) {
                    credit_boss_kill_stats(game, mvp);
                    char const *mob_nm = MOB_DATA[ent.get_mob_id()].name;
                    if (mob_nm == nullptr || mob_nm[0] == 0) mob_nm = "Boss";
                    std::string who = announce_party_names(game, mvp);
                    uint8_t is_party = mvp->party_id != 0;
                    std::string text = is_party
                        ? ("小队「" + who + "」合力击败了 " + mob_nm + "！")
                        : (who + " 击败了 " + mob_nm + "！");
                    uint8_t mid = (game != nullptr) ? game->map_id : sim->map_id;
                    Chat::broadcast_server("系统", text, {}, Chat::kKill, who, mid);
                }
            }

            // drop_team is the owner's camera id (personal, not shared party team).
            auto owner_has_bonus = [&](EntityID cam) -> uint8_t {
                Client *c = client_for_cam(cam);
                if (c == nullptr || c->account_name.empty()) return 0;
                return c->bonus_enabled
                    && (c->daily_bonus_seconds || c->offline_bonus_seconds) ? 1 : 0;
            };

            uint8_t const is_boss_kill =
                spawn_class(ent.get_mob_id()) == SpawnClass::kBoss;

            auto roll_for_owner = [&](EntityID owner_cam, float angle_off) {
                std::vector<PetalID::T> success_drops = {};
                auto roll_one = [&]() {
                    return _roll_mob_drop(sim, ent.get_mob_id(), live_rarity, is_boss_kill);
                };
                auto push_roll = [&]() {
                    if (success_drops.size() >= MAX_DROPS_PER_MOB) return;
                    PetalID::T extra = roll_one();
                    if (extra != PetalID::kNone)
                        success_drops.push_back(extra);
                };
                push_roll();
                push_roll();
                if (is_boss_kill) {
                    for (uint32_t n = 0; n < 4; ++n) push_roll();
                    if (frand() < 0.70f) push_roll();
                    if (frand() < 0.45f) push_roll();
                } else {
                    if (frand() < 0.72f) push_roll();
                    if (live_rarity >= RarityID::kUnusual && frand() < 0.55f)
                        push_roll();
                    if (live_rarity >= RarityID::kRare && frand() < 0.45f)
                        push_roll();
                    if (live_rarity >= RarityID::kEpic && frand() < 0.40f)
                        push_roll();
                    if (live_rarity >= RarityID::kLegendary && frand() < 0.35f)
                        push_roll();
                }
                if (owner_has_bonus(owner_cam)) {
                    size_t bonus_rolls = std::min<size_t>(success_drops.size() * 2, 12);
                    for (size_t i = 0; i < bonus_rolls; ++i) {
                        PetalID::T extra = roll_one();
                        if (extra != PetalID::kNone
                            && PETAL_DATA[extra].rarity != RarityID::kUnique)
                            success_drops.push_back(extra);
                    }
                }
                float ox = ent.get_x() + std::cos(angle_off) * 12.f;
                float oy = ent.get_y() + std::sin(angle_off) * 12.f;
                _alloc_drops(sim, success_drops, ox, oy, owner_cam);
                uint32_t coins = Craft::coins_from_mob(live_rarity);
                {
                    MapZones const &zones = map_zones(sim->map_id);
                    uint32_t diff = 0;
                    if (ent.zone < zones.size())
                        diff = zones[ent.zone].difficulty;
                    coins = (uint32_t) (coins * zone_coin_mult(diff)
                        * mob_spawn_xp_mult(mob_data.rarity, live_rarity) + 0.5f);
                    if (is_boss_kill) coins = coins * 3u + 80u;
                    if (coins < 1) coins = 1;
                }
                if (coins > 0) {
                    uint16_t pile = coins > 65535 ? (uint16_t) 65535 : (uint16_t) coins;
                    Entity &drop = alloc_drop(sim, PetalID::kCoin, owner_cam, pile);
                    float cx = ox, cy = oy;
                    Map::resolve_circle_terrain(sim->map_id, cx, cy, drop.get_radius() + 8.f);
                    cx = fclamp(cx, drop.get_radius(), ARENA_WIDTH - drop.get_radius());
                    cy = fclamp(cy, drop.get_radius(), ARENA_HEIGHT - drop.get_radius());
                    drop.set_x(cx);
                    drop.set_y(cy);
                    drop.velocity.unit_normal(frand() * 2 * (float) M_PI)
                        .set_magnitude(18 + frand() * 20);
                }
            };

            // No eligible player means no loot. Never create a public fallback
            // pile that a bystander/teammate can see, magnetize or take.
            for (uint8_t i = 0; i < recip_n; ++i) {
                float ang = (recip_n == 1) ? 0.f
                    : (float) i * (2.f * (float) M_PI / (float) recip_n);
                roll_for_owner(recipients[i], ang);
            }
        }
        if (ent.get_mob_id() == MobID::kAntHole && 
            BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone) && 
            frand() < DIGGER_SPAWN_CHANCE) { 
            EntityID team = NULL_ENTITY;
            if (sim->ent_exists(ent.last_damaged_by))
                team = sim->get_ent(ent.last_damaged_by).get_team();
            alloc_mob(sim, MobID::kDigger, ent.get_x(), ent.get_y(), team);
        }

    } else if (ent.has_component(kPetal)) {
        if (petal_name_is(ent.get_petal_id(), "Web"))
            alloc_web(sim, 100, ent);
    } else if (ent.has_component(kFlower)) {
        // Peaceful title leave (kNoDrops): full inventory keep, no ground drops / corpse
        if (BitMath::at(ent.flags, EntityFlags::kNoDrops) && sim->ent_exists(ent.get_parent())) {
            Entity &camera = sim->get_ent(ent.get_parent());
            // Drop petal tracker refs from the flower slots (camera already holds them).
            uint32_t const total_slots = ent.get_loadout_count() + MAX_SLOT_COUNT;
            for (uint32_t i = 0; i < total_slots && i < 2 * MAX_SLOT_COUNT; ++i)
                PetalTracker::remove_petal(sim, ent.get_loadout_ids(i));
            flush_flower_loadout_to_camera(sim, camera, ent);
            // Fill empty primary with Basics only for title-leave respawn layout.
            uint32_t primary = loadout_slots_at_level(camera.get_respawn_level());
            for (uint32_t i = 0; i < primary; ++i) {
                if (camera.get_inventory(i) != PetalID::kNone) continue;
                camera.set_inventory(i, PetalID::kBasic);
                PetalTracker::add_petal(sim, PetalID::kBasic);
            }
            camera.set_killed_by("");
            if (Client *owner = Server::client_by_camera(sim, camera.id)) {
                AccountStore::capture_nosave(owner);
                Party::clear_corpse(owner);
                owner->has_death_pos = 0;
                owner->play_name.clear();
            }
            return;
        }
        // No ground drops on player death — keep full loadout + bag.
        // Slot layout preserved for respawn (camera[0..loadout)=primary, rest=bag).
        uint32_t const total_slots = ent.get_loadout_count() + MAX_SLOT_COUNT;
        std::vector<PetalID::T> slot_petals(total_slots, PetalID::kNone);
        for (uint32_t i = 0; i < total_slots; ++i) {
            DEBUG_ONLY(assert(ent.get_loadout_ids(i) < PetalID::kNumPetals));
            PetalID::T id = ent.get_loadout_ids(i);
            PetalTracker::remove_petal(sim, id);
            slot_petals[i] = id;
        }
        std::vector<PetalID::T> deleted_keep = {};
        for (uint32_t i = 0; i < ent.deleted_petals.size(); ++i) {
            DEBUG_ONLY(assert(ent.deleted_petals[i] < PetalID::kNumPetals));
            PetalTracker::remove_petal(sim, ent.deleted_petals[i]);
            if (ent.deleted_petals[i] != PetalID::kNone && ent.deleted_petals[i] != PetalID::kBasic)
                deleted_keep.push_back(ent.deleted_petals[i]);
        }
        if (!sim->ent_alive(ent.get_parent()))
            return;
        Entity &camera = sim->get_ent(ent.get_parent());
        float death_x = ent.get_x();
        float death_y = ent.get_y();
        // Level never regresses on death — keep floor + mid-level XP.
        uint32_t respawn_level = score_to_level(ent.get_score());
        if (respawn_level < 1) respawn_level = 1;
        if (respawn_level < camera.get_respawn_level())
            respawn_level = camera.get_respawn_level();
        camera.set_respawn_level(respawn_level);
        if (Client *owner = Server::client_by_camera(sim, camera.id)) {
            uint32_t sc = ent.get_score();
            uint32_t floor = level_to_score(respawn_level);
            if (sc < floor) sc = floor;
            AccountStore::note_score(owner, sc);
        }
        uint32_t primary_slots = loadout_slots_at_level(respawn_level);

        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
            camera.set_inventory(i, PetalID::kNone);

        auto put_in_backpack = [&](PetalID::T id) {
            if (id == PetalID::kNone) return;
            uint32_t dest = primary_slots;
            while (dest < primary_slots + MAX_SLOT_COUNT
                && dest < 2 * MAX_SLOT_COUNT
                && camera.get_inventory(dest) != PetalID::kNone)
                ++dest;
            if (dest >= primary_slots + MAX_SLOT_COUNT || dest >= 2 * MAX_SLOT_COUNT)
                return;
            PetalTracker::add_petal(sim, id);
            camera.set_inventory(dest, id);
        };

        // 1) Primary petals that still fit stay in primary (bag-only → backpack)
        for (uint32_t i = 0; i < ent.get_loadout_count() && i < primary_slots; ++i) {
            if (slot_petals[i] == PetalID::kNone) continue;
            if (Recipe::is_bag_only(slot_petals[i])) {
                put_in_backpack(slot_petals[i]);
                continue;
            }
            PetalTracker::add_petal(sim, slot_petals[i]);
            camera.set_inventory(i, slot_petals[i]);
        }
        // 2) Overflow primary + backpack + deleted → backpack (order preserved)
        for (uint32_t i = primary_slots; i < ent.get_loadout_count(); ++i)
            put_in_backpack(slot_petals[i]);
        for (uint32_t i = ent.get_loadout_count(); i < total_slots; ++i)
            put_in_backpack(slot_petals[i]);
        for (PetalID::T id : deleted_keep)
            put_in_backpack(id);
        // 3) Fill empty primary with Basics
        for (uint32_t i = 0; i < primary_slots; ++i) {
            if (camera.get_inventory(i) != PetalID::kNone) continue;
            PetalTracker::add_petal(sim, PetalID::kBasic);
            camera.set_inventory(i, PetalID::kBasic);
        }
        if (Client *owner = Server::client_by_camera(sim, camera.id)) {
            // Nosave: sync write of accounts.dat on death stalls the whole server tick
            AccountStore::capture_nosave(owner);
            client_send_life_summary(owner);
            Party::on_death(owner, death_x, death_y, ent.get_color(), ent.get_equip_flags());
        }
    } else if (ent.has_component(kDrop)) {
        if (BitMath::at(ent.flags, EntityFlags::kIsDespawning))
            PetalTracker::remove_petal(sim, ent.get_drop_id());
    }
}
