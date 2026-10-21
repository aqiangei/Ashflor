#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>

#include <Client/Assets/Assets.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Particle.hh>
#include <Client/StaticData.hh>
#include <Client/Ui/Ui.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <cmath>
#include <unordered_set>

// Rebuild once per frame while G is held — avoid per-petal parent lookups.
static double g_tier_stamp = -1.0;
static uint8_t g_tier_on = 0;
static std::unordered_set<EntityID> g_other_flower_ids;

static void refresh_tier_overlay_state() {
    if (g_tier_stamp == Game::timestamp) return;
    g_tier_stamp = Game::timestamp;
    g_tier_on = Input::keys_held.contains('G') ? 1 : 0;
    g_other_flower_ids.clear();
    if (!g_tier_on) return;
    Game::simulation.for_each<kFlower>([](Simulation *, Entity const &flower) {
        if (flower.has_component(kMob)) return;
        if (Game::simulation.ent_alive(Game::player_id)
            && flower.id == Game::player_id)
            return;
        g_other_flower_ids.insert(flower.id);
    });
}

static uint8_t other_player_petal(Entity const &ent) {
    EntityID const pid = ent.get_parent();
    if (pid == NULL_ENTITY) return 0;
    return g_other_flower_ids.contains(pid) ? 1 : 0;
}

void render_petal(Renderer &ctx, Entity const &ent) {
    PetalID::T const id = ent.get_petal_id();
    if (id >= PetalID::kNumPetals) return;
    struct PetalData const &data = PETAL_DATA[id];

    refresh_tier_overlay_state();
    uint8_t const rarity = data.rarity;
    uint8_t const show_tier = g_tier_on
        && rarity < RarityID::kNumRarities
        && other_player_petal(ent);

    // Rooted Monstera: soft green heal disc under the leaf (world units, before petal scale).
    if (data.attributes.aura_heal > 0 && ent.get_split_projectile()
        && data.attributes.aura_radius > 0) {
        RenderContext aura(&ctx);
        float const R = data.attributes.aura_radius;
        ctx.set_fill(0x2828A860);
        ctx.begin_path();
        ctx.arc(0, 0, R);
        ctx.fill();
        ctx.set_fill(0x1838B870);
        ctx.begin_path();
        ctx.arc(0, 0, R * 0.55f);
        ctx.fill();
        ctx.set_stroke(0x5540C878);
        ctx.set_line_width(2.5f);
        ctx.begin_path();
        ctx.arc(0, 0, R);
        ctx.stroke();
    }

    // One soft rarity disc — no stroke / multi-pass (those tank Canvas2D with G held).
    if (show_tier) {
        float const r = ent.get_radius();
        uint32_t const col = RARITY_COLORS[rarity];
        ctx.set_fill((col & 0x00ffffff) | 0xaa000000);
        ctx.begin_path();
        ctx.arc(0, 0, r * 1.35f);
        ctx.fill();
    }

    ctx.scale(ent.get_radius() / data.radius);
    // Monstera reuses split_projectile as "rooted" flag — always draw a single leaf.
    if (ent.get_split_projectile() && id != PetalID::kMonstera)
        draw_static_petal(id, ctx);
    else
        draw_static_petal_single(id, ctx);
    if (data.rarity == RarityID::kUnique && frand() < fclamp(0.25 * Ui::dt/16.67, 0, 1))
        Particle::add_unique_particle(ent.get_x(), ent.get_y());

    // Fill-only label (strokeText is the main hitch). Cap count via small petals skip.
    if (show_tier && ent.get_radius() >= 8.f) {
        RenderContext label(&ctx);
        float const s = ent.get_radius() / data.radius;
        if (s > 1e-3f) ctx.scale(1.f / s);
        ctx.rotate(-ent.get_angle());
        ctx.translate(0, ent.get_radius() + 11.f);
        ctx.draw_text(I18n::rarity_name(rarity), {
            .fill = RARITY_COLORS[rarity] | 0xff000000,
            .stroke = 0,
            .size = 11,
            .stroke_scale = 0
        });
    }
}
