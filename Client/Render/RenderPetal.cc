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
static std::unordered_set<EntityID> g_player_flower_ids;

static void refresh_tier_overlay_state() {
    if (g_tier_stamp == Game::timestamp) return;
    g_tier_stamp = Game::timestamp;
    g_tier_on = Input::keys_held.contains('G') ? 1 : 0;
    g_player_flower_ids.clear();
    if (!g_tier_on) return;
    // All player flowers in the local sim (AOI ≈ nearby), including self.
    Game::simulation.for_each<kFlower>([](Simulation *, Entity &ent){
        if (ent.has_component(kMob)) return;
        g_player_flower_ids.insert(ent.id);
    });
}

static uint8_t player_owned_petal(Entity const &ent) {
    EntityID const pid = ent.get_parent();
    if (pid == NULL_ENTITY) return 0;
    return g_player_flower_ids.contains(pid) ? 1 : 0;
}

void render_petal(Renderer &ctx, Entity const &ent) {
    PetalID::T const id = ent.get_petal_id();
    if (id >= PetalID::kNumPetals) return;
    struct PetalData const &data = PETAL_DATA[id];

    refresh_tier_overlay_state();
    uint8_t const rarity = data.rarity;
    uint8_t const show_tier = g_tier_on
        && rarity < RarityID::kNumRarities
        && player_owned_petal(ent);

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
    // Highest rarity is Unique — sparkle only for Unique petals (not Super/Ultra).
    // Higher account level → denser, more golden particles.
    if (data.rarity == RarityID::kUnique) {
        uint32_t lv = (id < PetalID::kNumPetals) ? Game::petal_levels[id] : 0;
        float rate = 0.22f + fclamp((float) lv * 0.012f, 0.f, 0.35f);
        if (frand() < fclamp(rate * Ui::dt / 16.67, 0, 1))
            Particle::add_unique_particle(ent.get_x(), ent.get_y(), lv);
    }

    // Fill-only label (strokeText is the main hitch). Cap count via small petals skip.
    if (show_tier && ent.get_radius() >= 8.f) {
        RenderContext label(&ctx);
        float const s = ent.get_radius() / data.radius;
        if (s > 1e-3f) ctx.scale(1.f / s);
        ctx.rotate(-ent.get_angle());
        ctx.translate(0, ent.get_radius() + 11.f);
        // Unique charcoal needs a light rim so "唯一" stays readable in the field.
        uint32_t const col = RARITY_COLORS[rarity] | 0xff000000;
        uint8_t const unique = (rarity == RarityID::kUnique) ? 1 : 0;
        ctx.draw_text(I18n::rarity_name(rarity), {
            .fill = col,
            .stroke = unique ? 0xffe8eaee : 0,
            .size = 11,
            .stroke_scale = unique ? 0.16f : 0
        });
    }
}
