#include <Client/Particle.hh>

#include <Client/Game.hh>

#include <Client/Assets/Assets.hh>

#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <array>
#include <cmath>
#include <vector>

using namespace Particle;

static std::vector<TitleParticleEntity> title_particles;
static std::vector<GameParticleEntity> game_particles;

// Petals that drop from mobs spawning on each map (biome-flavored title flybys).
static std::array<std::vector<PetalID::T>, MapID::kNumMaps> map_title_petals;
static uint8_t map_title_petals_ready = 0;

static void ensure_map_title_petals() {
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

static std::vector<PetalID::T> const &title_petal_pool_for_selected_map() {
    ensure_map_title_petals();
    uint8_t mid = Game::selected_map;
    if (mid >= MapID::kNumMaps) mid = MapID::kGarden;
    return map_title_petals[mid];
}

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
    // Prefer collected petals from this biome; fall back to full biome pool so
    // the flyby still matches the selected map on a fresh account.
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
        part.opacity = fclamp(part.opacity - dt / 1000, 0, 1);
        ctx.set_global_alpha(part.opacity);
        ctx.set_fill(0x80ffffff);
        ctx.begin_path();
        ctx.arc(part.x,part.y,part.radius);
        ctx.fill();
    }
}

void Particle::add_unique_particle(float x, float y) {
    GameParticleEntity part;
    part.x = x;
    part.y = y;
    part.radius = 4;
    part.opacity = 1;
    Vector rand = Vector::rand(50);
    part.x_velocity = rand.x;
    part.y_velocity = rand.y;
    game_particles.push_back(std::move(part));
}