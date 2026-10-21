#include <Client/Particle.hh>

#include <Client/Game.hh>
#include <Client/Ui/Extern.hh>

#include <Client/Assets/Assets.hh>

#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

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
        part.opacity = fclamp(part.opacity - dt / 1000, 0, 1);
        ctx.set_global_alpha(part.opacity);
        ctx.set_fill(0x80ffffff);
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
            .stroke_scale = 0.18f
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
