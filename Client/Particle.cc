#include <Client/Particle.hh>

#include <Client/Game.hh>
#include <Client/ThemeMaterial.hh>
#include <Client/Ui/Extern.hh>

#include <Client/Assets/Assets.hh>

#include <Shared/Entity.hh>
#include <Shared/RarityScale.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/TreasureSites.hh>

#include <Helpers/Math.hh>
#include <Helpers/Vector.hh>

#include <array>
#include <cmath>
#include <format>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Particle;

namespace {

struct FloatingText {
    float x;
    float y;
    float vy;
    float life; // seconds remaining
    float size;
    uint32_t fill;
    std::string text;
};

std::vector<TitleParticleEntity> title_particles;
std::vector<GameParticleEntity> game_particles;
std::vector<FloatingText> floating_texts;

// Throttle / accumulate damage pops per mob (ms).
constexpr double kDmgCooldownMs = 90.0;
struct DmgBucket {
    float amount = 0;
    float x = 0;
    float y = 0;
    double last_show_ms = -1e9;
};
std::unordered_map<uint32_t, DmgBucket> dmg_buckets;

// Petals that drop from mobs spawning on each map (biome-flavored title flybys).
std::array<std::vector<PetalID::T>, MapID::kNumMaps> map_title_petals;
uint8_t map_title_petals_ready = 0;

void ensure_map_title_petals() {
    if (map_title_petals_ready) return;
    for (uint8_t mid = 0; mid < MapID::kNumMaps; ++mid) {
        std::array<uint8_t, PetalID::kNumPetals> seen{};
        std::vector<PetalID::T> &out = map_title_petals[mid];
        out.clear();
        out.push_back(PetalID::kBasic);
        seen[PetalID::kBasic] = 1;
        MapZones const &zones = map_zones(mid);
        for (uint32_t zi = 0; zi < zones.size(); ++zi) {
            ZoneDefinition const &zone = zones[zi];
            for (uint32_t si = 0; si < zone.spawns.size(); ++si) {
                MobID::T mob = zone.spawns[si].id;
                if (mob >= MobID::kNumMobs) continue;
                auto const &drops = MOB_DATA[mob].drops;
                for (uint32_t di = 0; di < drops.size(); ++di) {
                    PetalID::T pid = drops[di];
                    if (pid >= PetalID::kNumPetals || seen[pid]) continue;
                    seen[pid] = 1;
                    out.push_back(pid);
                }
            }
        }
    }
    map_title_petals_ready = 1;
}

std::vector<PetalID::T> const &title_petal_pool_for_selected_map() {
    ensure_map_title_petals();
    uint8_t mid = Game::selected_map;
    if (mid >= MapID::kNumMaps) mid = MapID::kGarden;
    return map_title_petals[mid];
}

void spawn_floating(float x, float y, std::string text, uint32_t fill, float size) {
    FloatingText t;
    t.x = x + (float)(frand() * 18.0 - 9.0);
    t.y = y;
    t.vy = -55.f - (float)frand() * 25.f;
    t.life = 0.85f;
    t.size = size;
    t.fill = fill;
    t.text = std::move(text);
    floating_texts.push_back(std::move(t));
}

void flush_damage(uint32_t key, DmgBucket &b) {
    if (b.amount < 0.5f) {
        b.amount = 0;
        return;
    }
    int shown = (int)std::lround(b.amount);
    if (shown < 1) shown = 1;
    spawn_floating(b.x, b.y, std::format("{}", shown), 0xffff4a4a, 18.f);
    b.amount = 0;
    b.last_show_ms = Game::timestamp;
}

} // namespace

void Particle::tick_title(Renderer &ctx, double dt) {
    RenderContext c(&ctx);
    ctx.reset_transform();
    size_t len = title_particles.size();
    for (size_t i = len; i > 0; --i) {
        TitleParticleEntity &part = title_particles[i - 1];
        if (part.x > ctx.width + std::max(10.0f, PETAL_DATA[part.id].radius) * part.radius) {
            part = title_particles[title_particles.size() - 1];
            title_particles.pop_back();
            continue;
        }
        RenderContext c(&ctx);
        part.x += part.x_velocity * (dt / 1000) * Ui::scale;
        part.angle += dt / 1000 * Ui::scale;
        ctx.translate(part.x, part.y + 12.5 * sin(Game::timestamp / 500 + part.sin_offset));
        ctx.scale(Ui::scale * part.radius);
        if (PETAL_DATA[part.id].attributes.rotation_style == PetalAttributes::kPassiveRot)
            ctx.rotate(part.angle);
        if (part.id == PetalID::kPeas || part.id == PetalID::kPoisonPeas)
            draw_static_petal(part.id, ctx);
        else
            draw_static_petal_single(part.id, ctx);
    }

    std::vector<PetalID::T> const &pool = title_petal_pool_for_selected_map();
    std::vector<PetalID::T> ids;
    float freq_sum = 0;
    for (PetalID::T id : pool) {
        if (!Game::seen_petals[id] && id != PetalID::kBasic) continue;
        ids.push_back(id);
        freq_sum += (float)pow(0.5, PETAL_DATA[id].rarity);
    }
    if (ids.size() <= 1) {
        ids = pool;
        freq_sum = 0;
        for (PetalID::T id : ids)
            freq_sum += (float)pow(0.5, PETAL_DATA[id].rarity);
    }
    if (freq_sum <= 0) {
        ids = {PetalID::kBasic};
        freq_sum = 1;
    }

    for (size_t i = 0; i < 4; ++i) {
        if (frand() > 0.02) continue;
        TitleParticleEntity npart;
        npart.x = -100;

        float freq_score = freq_sum * frand();
        for (PetalID::T id : ids) {
            freq_score -= (float)pow(0.5, PETAL_DATA[id].rarity);
            if (freq_score > 0) continue;
            npart.id = id;
            npart.y = frand() * ctx.height;
            npart.angle = frand() * 2 * M_PI;
            npart.x_velocity = frand() * 100 + 100;
            npart.sin_offset = frand() * M_PI;
            npart.radius = frand() + 0.5;
            title_particles.push_back(std::move(npart));
            break;
        }
    }
}

void Particle::tick_game(Renderer &ctx, double dt) {
    size_t len = game_particles.size();
    for (size_t i = len; i > 0; --i) {
        GameParticleEntity &part = game_particles[i - 1];
        if (part.opacity < 0.1) {
            part = game_particles[game_particles.size() - 1];
            game_particles.pop_back();
            continue;
        }
        RenderContext c(&ctx);
        part.x += part.x_velocity * dt / 1000;
        part.y += part.y_velocity * dt / 1000;
        // Spirit motes: same read as Unique petal sparkles (bright + halo).
        uint8_t spirit = (part.fill == 0xfffff6d0 || part.fill == 0xffffd060
            || part.fill == 0xffffffff || part.fill == 0xffffc84a
            || part.fill == 0xffff8ad8);
        float fade = spirit ? (float) (dt / 1600.0) : (float) (dt / 1000.0);
        part.opacity = fclamp(part.opacity - fade, 0, 1);
        ctx.set_global_alpha(part.opacity);
        if (spirit) {
            ctx.set_fill(0x90ffe08a);
            ctx.begin_path();
            ctx.arc(part.x, part.y, part.radius * 2.4f);
            ctx.fill();
        }
        ctx.set_fill(part.fill);
        ctx.begin_path();
        ctx.arc(part.x, part.y, part.radius);
        ctx.fill();
    }

    float const sec = (float)(dt / 1000.0);
    for (size_t i = floating_texts.size(); i > 0; --i) {
        FloatingText &t = floating_texts[i - 1];
        t.life -= sec;
        if (t.life <= 0.f) {
            t = floating_texts.back();
            floating_texts.pop_back();
            continue;
        }
        t.y += t.vy * sec;
        t.vy *= (1.f - 2.2f * sec); // ease up
        float alpha = t.life > 0.35f ? 1.f : fclamp(t.life / 0.35f, 0.f, 1.f);
        RenderContext c(&ctx);
        ctx.set_global_alpha(alpha);
        ctx.translate(t.x, t.y);
        ctx.draw_text(t.text.c_str(), {
            .fill = t.fill,
            .stroke = 0xff000000,
            .size = t.size,
            // Fill-only: strokeText on every pop was expensive in big fights.
            .stroke_scale = 0.f
        });
    }

    // Flush damage buckets whose cooldown has elapsed (last hit after window).
    for (auto it = dmg_buckets.begin(); it != dmg_buckets.end(); ) {
        DmgBucket &b = it->second;
        if (b.amount >= 0.5f && Game::timestamp - b.last_show_ms >= kDmgCooldownMs)
            flush_damage(it->first, b);
        if (b.amount < 0.5f && Game::timestamp - b.last_show_ms > 2000.0)
            it = dmg_buckets.erase(it);
        else
            ++it;
    }

    tick_guide_sprite(dt);
    tick_ambient_env(dt);
}

void Particle::tick_ambient_env(double dt) {
    // Pure visual — does not touch Unique petal sparkles (add_unique_particle).
    if (!Game::alive()) return;
    if (!Game::simulation.ent_alive(Game::player_id)) return;
    // P3: ambient yields earlier so escort trails / Unique FX keep headroom.
    if (game_particles.size() > 40) return;
    Entity const &player = Game::simulation.get_ent(Game::player_id);
    if (Game::timestamp - player.last_damaged_time < 400) return;

    uint8_t mid = Game::simulation.arena_info.map_id;
    if (mid >= MapID::kNumMaps) return;

    static double ambient_cd_ms = 0;
    static double burst_left_ms = 0;
    ambient_cd_ms -= dt;
    burst_left_ms -= dt;

    // Themed env bursts (G04 pollen / D06 dust / O03 tide / S03 snow / V05 ash…)
    if (burst_left_ms <= 0 && ambient_cd_ms <= 0) {
        burst_left_ms = 10000 + frand() * 4000;
        ambient_cd_ms = 35000 + frand() * 25000;
    }
    if (burst_left_ms <= 0) return;

    static double spawn_cd_ms = 0;
    spawn_cd_ms -= dt;
    if (spawn_cd_ms > 0) return;
    spawn_cd_ms = 200 + frand() * 180;

    float px = player.get_x(), py = player.get_y();
    uint32_t col = 0xb0F6E9B5;
    float drift_x = 18.f, drift_y = -12.f;
    float rad = 2.2f;
    float spread = 280.f;
    uint8_t n = 3;
    switch (mid) {
    case MapID::kGarden: // G04 花粉风
        col = 0xb0F6E9B5;
        drift_x = 26.f; drift_y = -18.f;
        rad = 2.5f; n = 4; spread = 320.f;
        break;
    case MapID::kDesert: // D06 沙尘横扫
        col = 0xa0E6BE79;
        drift_x = 48.f; drift_y = -2.f;
        rad = 1.8f; n = 5; spread = 360.f;
        break;
    case MapID::kOcean: // O03 潮汐光纹
        col = 0xa04CC9F0;
        drift_x = 6.f; drift_y = -28.f;
        rad = 2.8f; n = 4; spread = 300.f;
        break;
    case MapID::kPolar: // S03 暴风雪前兆
        col = 0xc0FFFFFF;
        drift_x = 14.f; drift_y = 22.f;
        rad = 1.7f; n = 5; spread = 340.f;
        break;
    case MapID::kJungle: // J03 萤火虫
        col = 0xb095D5B2;
        drift_x = (frand() - 0.5f) * 20.f; drift_y = (frand() - 0.5f) * 20.f;
        rad = 2.0f; n = 4; spread = 260.f;
        break;
    case MapID::kAntHell: // A04 地道灰尘
        col = 0xa0D9C09B;
        drift_x = -12.f; drift_y = 8.f;
        rad = 1.6f; n = 3; spread = 240.f;
        break;
    case MapID::kVolcano: // V05 灰烬飘落 / V02 气泡感
        col = (frand() > 0.45f) ? 0xb0E76F51 : 0xa0373645;
        drift_x = 3.f; drift_y = -32.f;
        rad = 2.2f; n = 4; spread = 300.f;
        break;
    case MapID::kRoyalCapital: // 锈屑 / 旧游行尘
        col = 0xa08B8D98;
        drift_x = 16.f; drift_y = -6.f;
        rad = 1.9f; n = 3; spread = 280.f;
        break;
    default: return;
    }

    for (uint8_t i = 0; i < n; ++i) {
        GameParticleEntity part;
        part.x = px + (frand() - 0.5f) * spread;
        part.y = py + (frand() - 0.5f) * (spread * 0.7f);
        part.radius = rad + frand() * 1.5f;
        part.opacity = 0.5f + frand() * 0.3f;
        part.fill = col;
        part.x_velocity = drift_x + (frand() - 0.5f) * 16.f;
        part.y_velocity = drift_y + (frand() - 0.5f) * 12.f;
        game_particles.push_back(std::move(part));
    }
}

void Particle::tick_guide_sprite(double dt) {
    // G07: flower-spirit trail toward unvisited Yahaha / treasure / guide POIs.
    // Client-only — hard-capped; never touches Unique petal FX.
    if (!Game::alive()) return;
    if (!Game::simulation.ent_alive(Game::player_id)) return;
    if (game_particles.size() > 64) return;
    Entity const &player = Game::simulation.get_ent(Game::player_id);
    uint8_t mid = Game::simulation.arena_info.map_id;
    if (mid >= MapID::kNumMaps) return;

    static uint8_t found_mask[MapID::kNumMaps] = {};
    static double spark_cd_ms = 0;
    static float spirit_x = 0, spirit_y = 0;
    static uint8_t spirit_alive = 0;
    spark_cd_ms -= dt;

    if (Game::timestamp - player.last_damaged_time < 500) {
        spark_cd_ms = 800;
        spirit_alive = 0;
        return;
    }

    TreasureSites::MapPack pack = TreasureSites::pack(mid);
    float px = player.get_x(), py = player.get_y();

    // Priority: Ritual > ChestOnly > Guide. Skip sites already walked up to.
    int best = -1;
    float best_score = 1e30f;
    float best_tx = 0, best_ty = 0;
    for (uint8_t i = 0; i < pack.site_count; ++i) {
        TreasureSites::SiteDef const &s = pack.sites[i];
        if (found_mask[mid] & (1u << (i & 7))) continue;
        float tx, ty;
        TreasureSites::hub_xy(s.ox, s.oy, tx, ty);
        float dx = tx - px, dy = ty - py;
        float d2 = dx * dx + dy * dy;
        if (d2 < 380.f * 380.f) {
            found_mask[mid] |= (uint8_t) (1u << (i & 7));
            continue;
        }
        if (d2 < 700.f * 700.f) continue; // too close to bother guiding
        if (d2 > 2800.f * 2800.f) continue;
        float score = d2;
        if (s.kind == TreasureSites::kRitual) score *= 0.55f;
        else if (s.kind == TreasureSites::kChestOnly) score *= 0.85f;
        else score *= 1.15f; // Guide NPC — still valid (G07)
        if (score < best_score) {
            best_score = score;
            best = (int) i;
            best_tx = tx;
            best_ty = ty;
        }
    }
    if (best < 0) {
        spirit_alive = 0;
        spark_cd_ms = 2000;
        return;
    }

    float dx = best_tx - px, dy = best_ty - py;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.f) return;
    float ux = dx / len, uy = dy / len;

    // Soft spirit mote that drifts toward the POI (one persistent lead)
    if (!spirit_alive) {
        spirit_x = px + ux * 90.f;
        spirit_y = py + uy * 90.f;
        spirit_alive = 1;
    }
    float sx = best_tx - spirit_x, sy = best_ty - spirit_y;
    float sl = std::sqrt(sx * sx + sy * sy);
    if (sl > 1.f) {
        float step = (float) (dt * 0.085); // ~85 units/sec toward target
        if (step > sl) step = sl;
        spirit_x += sx / sl * step;
        spirit_y += sy / sl * step;
    }
    // Keep spirit roughly ahead of the player along the path
    float to_spirit_x = spirit_x - px, to_spirit_y = spirit_y - py;
    float tsl = std::sqrt(to_spirit_x * to_spirit_x + to_spirit_y * to_spirit_y);
    if (tsl > 420.f) {
        spirit_x = px + ux * 140.f;
        spirit_y = py + uy * 140.f;
    }

    uint32_t col = (TerrainArt::theme(mid).ground_light & 0x00ffffff) | 0xc0000000;
    // Lead spirit (always when under particle budget)
    if (game_particles.size() < 56) {
        GameParticleEntity lead;
        lead.x = spirit_x + (frand() - 0.5f) * 6.f;
        lead.y = spirit_y + (frand() - 0.5f) * 6.f;
        lead.radius = 4.5f + frand() * 1.5f;
        lead.opacity = 0.85f;
        lead.fill = col;
        lead.x_velocity = ux * 12.f;
        lead.y_velocity = uy * 12.f;
        game_particles.push_back(std::move(lead));
    }

    if (spark_cd_ms > 0) return;
    // Trail crumbs along player → spirit
    for (int n = 0; n < 2; ++n) {
        float along = 60.f + n * 55.f + frand() * 12.f;
        GameParticleEntity part;
        part.x = px + ux * along + (frand() - 0.5f) * 16.f;
        part.y = py + uy * along + (frand() - 0.5f) * 16.f;
        part.radius = 2.5f + frand() * 2.f;
        part.opacity = 0.7f;
        part.fill = col;
        part.x_velocity = ux * 28.f;
        part.y_velocity = uy * 28.f;
        game_particles.push_back(std::move(part));
    }
    spark_cd_ms = 900 + frand() * 500;
}

void Particle::add_unique_particle(float x, float y) {
    // Top-of-ladder Unique petal sparkle (not Super mint — Unique pink-red).
    GameParticleEntity part;
    part.x = x;
    part.y = y;
    part.radius = 4;
    part.opacity = 1;
    part.fill = (RARITY_COLORS[RarityID::kUnique] & 0x00ffffff) | 0xb0000000;
    Vector rand = Vector::rand(50);
    part.x_velocity = rand.x;
    part.y_velocity = rand.y;
    game_particles.push_back(std::move(part));
}

size_t Particle::game_particle_count() {
    return game_particles.size();
}

void Particle::add_spirit_trail(float x, float y, float back_dx, float back_dy) {
    // Match Unique-petal readability: big, opaque, constant sparkle + rear ribbon.
    if (game_particles.size() > 280) return;
    float blen = std::sqrt(back_dx * back_dx + back_dy * back_dy);
    float ux = 0.f, uy = 0.f;
    if (blen > 1e-3f) {
        ux = back_dx / blen;
        uy = back_dy / blen;
    } else {
        // Idle / first frame — still scatter outward so trail is never invisible.
        Vector rdir = Vector::rand(1);
        ux = rdir.x;
        uy = rdir.y;
    }
    // Bright solids (full alpha) — Unique pink + gold/white petal tones.
    static uint32_t const kCols[5] = {
        0xffffffff, 0xfffff6d0, 0xffffd060, 0xffffc84a, 0xffff8ad8
    };

    // 1) Body sparkles like Unique petals (scatter around the spirit).
    for (int i = 0; i < 4; ++i) {
        GameParticleEntity part;
        Vector rand = Vector::rand(70);
        part.x = x + (frand() - 0.5f) * 22.f;
        part.y = y + (frand() - 0.5f) * 22.f;
        part.radius = 5.5f + frand() * 5.f;
        part.opacity = 1.f;
        part.fill = kCols[(int) (frand() * 5.f) % 5];
        part.x_velocity = rand.x;
        part.y_velocity = rand.y;
        game_particles.push_back(std::move(part));
    }

    // 2) Trail ribbon behind motion (still local to the spirit path).
    for (int i = 0; i < 5; ++i) {
        GameParticleEntity part;
        float back = 6.f + (float) i * 12.f + frand() * 10.f;
        float side = (frand() - 0.5f) * 18.f;
        float px = -uy * side, py = ux * side;
        part.x = x + ux * back + px;
        part.y = y + uy * back + py;
        part.radius = 7.f + frand() * 8.f;
        part.opacity = 1.f;
        part.fill = kCols[i % 5];
        part.x_velocity = ux * (24.f + frand() * 20.f) + (frand() - 0.5f) * 10.f;
        part.y_velocity = uy * (24.f + frand() * 20.f) + (frand() - 0.5f) * 10.f;
        game_particles.push_back(std::move(part));
    }
}

void Particle::add_damage_number(EntityID const &id, float x, float y, float amount) {
    if (amount < 0.5f || id.null()) return;
    uint32_t key = EntityID::make_hash(id);
    DmgBucket &b = dmg_buckets[key];
    b.amount += amount;
    b.x = x;
    b.y = y;
    if (Game::timestamp - b.last_show_ms >= kDmgCooldownMs)
        flush_damage(key, b);
}

void Particle::add_heal_number(float x, float y, float amount) {
    if (amount < 0.5f) return;
    int shown = (int)std::lround(amount);
    if (shown < 1) shown = 1;
    spawn_floating(x, y - 8.f, std::format("+{}", shown), 0xffff96cb, 17.f);
}

void Particle::clear_combat_accum(EntityID const &id) {
    if (id.null()) return;
    uint32_t key = EntityID::make_hash(id);
    auto it = dmg_buckets.find(key);
    if (it == dmg_buckets.end()) return;
    if (it->second.amount >= 0.5f)
        flush_damage(key, it->second);
    dmg_buckets.erase(it);
}
