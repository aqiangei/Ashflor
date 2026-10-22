#include <Client/Render/RenderEntity.hh>

#include <Client/Render/Renderer.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/StaticData.hh>

#include <Helpers/UTF8.hh>
#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

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

// Wrap speech UTF-8-safely into up to max_lines (by pixel width). Last line
// gets "…" if anything remains — never mid-codepoint byte cuts (broken 「�」).
static void wrap_speech(Renderer &ctx, std::string const &text, float max_w,
                        uint32_t max_lines, float size, std::vector<std::string> &out) {
    out.clear();
    if (text.empty() || max_lines == 0) return;
    ctx.set_text_size(size);
    UTF8Parser p(text);
    std::string cur;
    while (!p.at_end()) {
        uint32_t start = p.offset();
        if (p.next_symbol() == 0) break;
        uint32_t end = p.offset();
        std::string next = cur;
        next.append(text, start, end - start);
        if (!cur.empty() && ctx.get_text_size(next.c_str()) > max_w) {
            out.push_back(std::move(cur));
            cur.assign(text, start, end - start);
            if (out.size() + 1 >= max_lines) {
                // Pack remainder into the last line, ellipsize if needed.
                while (!p.at_end()) {
                    uint32_t s2 = p.offset();
                    if (p.next_symbol() == 0) break;
                    uint32_t e2 = p.offset();
                    std::string trial = cur;
                    trial.append(text, s2, e2 - s2);
                    trial += "…";
                    if (ctx.get_text_size(trial.c_str()) > max_w) {
                        cur += "…";
                        out.push_back(std::move(cur));
                        return;
                    }
                    cur.append(text, s2, e2 - s2);
                }
                out.push_back(std::move(cur));
                return;
            }
        } else {
            cur = std::move(next);
        }
    }
    if (!cur.empty()) out.push_back(std::move(cur));
}

void render_name(Renderer &ctx, Entity const &ent) {
    float rad = ent.get_radius();
    ctx.set_global_alpha(1 - ent.deletion_animation);
    ctx.scale(1 + 0.5 * ent.deletion_animation);

    // 1) Chat bubble above head (brief)
    if (ent.has_component(kName) && !ent.get_name().empty()) {
        auto it = Game::speech_bubbles.find(ent.get_name());
        if (it != Game::speech_bubbles.end() && it->second.expires_at > Game::timestamp) {
            float const size = 15.f;
            float const max_w = 240.f;
            std::vector<std::string> lines;
            wrap_speech(ctx, it->second.text, max_w, 3, size, lines);
            if (lines.empty()) lines.push_back(it->second.text);
            float remain = (float) (it->second.expires_at - Game::timestamp);
            float fade = remain < 800.f ? remain / 800.f : 1.f;
            RenderContext bubble(&ctx);
            ctx.set_text_size(size);
            float tw = 0.f;
            for (std::string const &ln : lines)
                tw = std::fmax(tw, ctx.get_text_size(ln.c_str()));
            float pad_x = 10.f;
            float pad_y = 6.f;
            float line_gap = 3.f;
            float bw = tw + pad_x * 2;
            float bh = size * (float) lines.size()
                + line_gap * (float) (lines.size() > 0 ? lines.size() - 1 : 0)
                + pad_y * 2;
            // Sit above the nametag (~-rad-20) so bubbles don't cover names.
            float bubble_y = -rad - 36.f - bh * 0.5f;
            ctx.translate(0, bubble_y);
            ctx.set_global_alpha((1 - ent.deletion_animation) * fade);
            ctx.set_fill(0xc0222222);
            ctx.begin_path();
            ctx.round_rect(-bw / 2, -bh / 2, bw, bh, 6);
            ctx.fill();
            // little tail
            ctx.begin_path();
            ctx.move_to(-5, bh / 2);
            ctx.line_to(0, bh / 2 + 8);
            ctx.line_to(5, bh / 2);
            ctx.close_path();
            ctx.fill();
            float y0 = -bh / 2 + pad_y + size * 0.5f;
            for (uint32_t i = 0; i < lines.size(); ++i) {
                RenderContext row(&ctx);
                ctx.translate(0, y0 + (float) i * (size + line_gap));
                draw_label(ctx, lines[i].c_str(), size, 0xffffffff);
            }
        }
    }

    // 2) Other players: name centered above head. Self hides name.
    // Level / guild sit under the HP bar (render_health), always on for others.
    if (ent.get_nametag_visible()
        && ent.has_component(kName)
        && ent.has_component(kFlower)
        && !ent.has_component(kMob)
        && !ent.get_name().empty())
    {
        uint8_t is_self = Game::simulation.ent_alive(Game::player_id)
            && (ent.id == Game::player_id);
        if (!is_self) {
            std::string const &name = ent.get_name();
            RenderContext label(&ctx);
            ctx.set_text_size(15);
            ctx.translate(0, -rad - 20.f);
            draw_label(ctx, name.c_str(), 15, 0xffffffff);
        }
    }

    // Treasure guide NPCs (named mobs, not flowers)
    if (ent.has_component(kMob) && ent.get_mob_id() == MobID::kTreasureNpc
        && ent.has_component(kName) && !ent.get_name().empty()) {
        RenderContext label(&ctx);
        ctx.set_text_size(14);
        ctx.translate(0, -rad - 18.f);
        draw_label(ctx, ent.get_name().c_str(), 14, 0xffffe08a);
    }

    // Mob name / rarity layout lives in render_health (horizontal when no HP bar).
}
