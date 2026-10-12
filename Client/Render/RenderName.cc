#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/StaticData.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <string>
#include <unordered_map>

static float cached_label_width(Renderer &ctx, std::string const &text, int size) {
    static std::unordered_map<std::string, float> widths;
    std::string key = std::to_string(size) + ':' + text;
    auto found = widths.find(key);
    if (found != widths.end()) return found->second;
    float width = ctx.get_text_size(text.c_str());
    if (widths.size() >= 256) widths.clear();
    widths.emplace(std::move(key), width);
    return width;
}

static void draw_label(Renderer &ctx, char const *text, float size, uint32_t fill) {
    ctx.draw_text(text, {
        .fill = fill,
        .stroke = 0xff000000,
        .size = size,
        .stroke_scale = 0.14f
    });
}

static std::string truncate_speech(std::string const &text, size_t max_chars) {
    if (text.size() <= max_chars) return text;
    return text.substr(0, max_chars - 2) + "..";
}

void render_name(Renderer &ctx, Entity const &ent) {
    float rad = ent.get_radius();
    ctx.set_global_alpha(1 - ent.deletion_animation);
    ctx.scale(1 + 0.5 * ent.deletion_animation);

    // 1) Chat bubble above head (brief)
    if (ent.has_component(kName) && !ent.get_name().empty()) {
        auto it = Game::speech_bubbles.find(ent.get_name());
        if (it != Game::speech_bubbles.end() && it->second.expires_at > Game::timestamp) {
            std::string shown = truncate_speech(it->second.text, 28);
            float remain = (float) (it->second.expires_at - Game::timestamp);
            float fade = remain < 800.f ? remain / 800.f : 1.f;
            RenderContext bubble(&ctx);
            ctx.translate(0, -rad - 28);
            ctx.set_global_alpha((1 - ent.deletion_animation) * fade);
            ctx.set_text_size(15);
            float tw = ctx.get_text_size(shown.c_str());
            float pad_x = 10.f;
            float pad_y = 6.f;
            float bw = tw + pad_x * 2;
            float bh = 15.f + pad_y * 2;
            ctx.set_fill(0xc0222222);
            ctx.begin_path();
            ctx.round_rect(-bw / 2, -bh / 2 - 2, bw, bh, 6);
            ctx.fill();
            // little tail
            ctx.begin_path();
            ctx.move_to(-5, bh / 2 - 2);
            ctx.line_to(0, bh / 2 + 8);
            ctx.line_to(5, bh / 2 - 2);
            ctx.close_path();
            ctx.fill();
            draw_label(ctx, shown.c_str(), 15, 0xffffffff);
        }
    }

    // 2) Player / corpse name — bottom-left of entity
    if (ent.get_nametag_visible()
        && ent.has_component(kName)
        && ent.has_component(kFlower)
        && !ent.has_component(kMob)
        && !ent.get_name().empty())
    {
        std::string const &name = ent.get_name();
        RenderContext label(&ctx);
        ctx.set_text_size(14);
        float tw = cached_label_width(ctx, name, 14);
        // Centered text: place center so left edge sits near -rad
        ctx.translate(-rad + tw * 0.5f, rad + 16);
        draw_label(ctx, name.c_str(), 14, 0xffffffff);
    }

    // 3) Monster rarity — bottom-right (heads only, not worm segments)
    if (ent.has_component(kMob) && !ent.has_component(kSegmented)) {
        MobID::T mid = ent.get_mob_id();
        if (mid < MobID::kNumMobs) {
            uint8_t rarity = MOB_DATA[mid].rarity;
            if (rarity < RarityID::kNumRarities) {
                char const *label = I18n::rarity_name(rarity);
                RenderContext rare(&ctx);
                ctx.set_text_size(13);
                float tw = cached_label_width(ctx, label, 13);
                ctx.translate(rad - tw * 0.5f, rad + 16);
                draw_label(ctx, label, 13, RARITY_COLORS[rarity]);
            }
        }
    }
}
