#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Game.hh>
#include <Client/Input.hh>
#include <Client/I18n.hh>
#include <Client/Ui/Extern.hh>

#include <Shared/Map.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Client/StaticData.hh>
#include <Shared/RarityScale.hh>
#include <Helpers/Math.hh>

#include <algorithm>
#include <cmath>
#include <format>
#include <memory>
#include <string>
#include <vector>

using namespace Ui;

namespace {
struct MinimapLayer {
    uint8_t map_id;
    float width;
    bool levels;
    std::unique_ptr<Renderer> canvas;
};

std::vector<MinimapLayer> minimap_layers;

Renderer &static_minimap(uint8_t map_id, float width, float height, bool levels) {
    // Snap width so float noise cannot miss the cache and rebuild terrain.
    float const wkey = std::round(width);
    for (MinimapLayer &layer : minimap_layers) {
        if (layer.map_id == map_id && layer.width == wkey && layer.levels == levels)
            return *layer.canvas;
    }
    // Drop oldest — never wipe the whole cache (that made G hitch every time).
    constexpr size_t kMaxLayers = 24;
    if (minimap_layers.size() >= kMaxLayers)
        minimap_layers.erase(minimap_layers.begin());
    minimap_layers.push_back({map_id, wkey, levels, std::make_unique<Renderer>()});
    Renderer &canvas = *minimap_layers.back().canvas;
    canvas.set_dimensions(wkey, height);
    canvas.reset();
    canvas.clear();
    float const sx = wkey / (float)ARENA_WIDTH;
    float const sy = height / (float)ARENA_HEIGHT;

    canvas.set_fill(0xffF8F8F4);
    canvas.fill_rect(0, 0, width, height);
    if (levels) {
        // Later, more specific zones overwrite broad rings. Obstacles are
        // painted afterward, so colour appears only on walkable space.
        // Colours = petal rarity (Common → Ultra).
        for (ZoneDefinition const &def : map_zones(map_id)) {
            uint8_t tier = (uint8_t)std::min<uint32_t>(def.difficulty, MAX_DIFFICULTY);
            canvas.set_fill(RARITY_COLORS[tier]);
            canvas.fill_rect(def.left * sx, def.top * sy,
                             (def.right - def.left) * sx, (def.bottom - def.top) * sy);
        }
    }
    float oxs[TERRAIN_OUTLINE_MAX];
    float oys[TERRAIN_OUTLINE_MAX];
    for (TerrainFeature const &f : map_terrain(map_id)) {
        if (levels && !(f.flags & TerrainFlags::kImpassable)) continue;
        uint32_t n = Map::terrain_outline(f, oxs, oys, TERRAIN_OUTLINE_MAX);
        if (n < 3) continue;
        canvas.set_fill((f.flags & TerrainFlags::kImpassable)
            ? 0xff08090B : 0xffF8F8F4);
        canvas.begin_path();
        canvas.move_to(oxs[0] * sx, oys[0] * sy);
        for (uint32_t i = 1; i < n; ++i)
            canvas.line_to(oxs[i] * sx, oys[i] * sy);
        canvas.close_path();
        canvas.fill();
    }
    return canvas;
}
} // namespace

// Full-arena aspect (ARENA_WIDTH : ARENA_HEIGHT), always 1:1 world replica
Minimap::Minimap(float w) : Element(w, w * ARENA_HEIGHT / (float)ARENA_WIDTH, {}) {}

void Minimap::refactor() {
    // Compact strip vs larger strip — both full-map, correct aspect
    width = Game::minimap_expanded ? 420.f : 280.f;
    height = width * ARENA_HEIGHT / (float)ARENA_WIDTH;
}

void Minimap::on_render(Renderer &ctx) {
    ctx.set_fill(0xe0111111);
    ctx.begin_path();
    ctx.round_rect(-width / 2, -height / 2, width, height, 12);
    ctx.fill();
    ctx.set_line_width(1.5f);
    ctx.set_stroke(0x66ffffff);
    ctx.begin_path();
    ctx.round_rect(-width / 2, -height / 2, width, height, 12);
    ctx.stroke();

    // Always full arena — 1:1 with world
    float const view_w = (float)ARENA_WIDTH;
    float const view_h = (float)ARENA_HEIGHT;
    float const left = 0.f;
    float const top = 0.f;
    float const sx = width / view_w;
    float const sy = height / view_h;

    {
        RenderContext clip(&ctx);
        ctx.begin_path();
        ctx.rect(-width / 2, -height / 2, width, height);
        ctx.clip();

        ctx.translate(-width / 2, -height / 2);
        uint8_t mid = Game::simulation.arena_info.map_id;
        if (mid >= MapID::kNumMaps) mid = MapID::kGarden;

        bool const show_levels = Input::keys_held.contains('G');
        bool const show_names = show_levels; // Hold G: rarity rings + player names on dots
        // Warm the levels layer in the background so first Hold-G does not hitch
        // on a full zone+terrain bake.
        if (!show_levels)
            (void)static_minimap(mid, width, height, true);
        ctx.draw_image_at(static_minimap(mid, width, height, show_levels), 0.f, 0.f);

        // Field portals for this map (Shared MAP_PORTALS — same table as server transfer).
        {
            float const pr = Game::minimap_expanded ? 6.5f : 5.2f;
            float const name_sz = Game::minimap_expanded ? 10.f : 8.5f;
            for (MapPortalDef const &p : MAP_PORTALS) {
                if (p.from_map != mid) continue;
                if (p.to_map >= MapID::kNumMaps || !map_is_open(p.to_map)) continue;
                float px = (p.x - left) * sx;
                float py = (p.y - top) * sy;
                // Diamond gate marker — cyan so it reads on rarity rings + terrain
                ctx.set_fill(0xeeffffff);
                ctx.begin_path();
                ctx.move_to(px, py - pr - 1.5f);
                ctx.line_to(px + pr + 1.5f, py);
                ctx.line_to(px, py + pr + 1.5f);
                ctx.line_to(px - pr - 1.5f, py);
                ctx.close_path();
                ctx.fill();
                ctx.set_fill(0xff2ee6c8);
                ctx.set_stroke(0xff0a5a52);
                ctx.set_line_width(1.4f);
                ctx.begin_path();
                ctx.move_to(px, py - pr);
                ctx.line_to(px + pr, py);
                ctx.line_to(px, py + pr);
                ctx.line_to(px - pr, py);
                ctx.close_path();
                ctx.fill();
                ctx.stroke();
                // Inner dot
                ctx.set_fill(0xfff0fffc);
                ctx.begin_path();
                ctx.arc(px, py, pr * 0.28f);
                ctx.fill();
                // Destination map name under the diamond
                {
                    RenderContext label(&ctx);
                    ctx.translate(px, py + pr + 9.f);
                    ctx.draw_text(I18n::map_name(p.to_map), {
                        .fill = 0xffd8fff8,
                        .stroke = 0,
                        .size = name_sz,
                        .stroke_scale = 0
                    });
                }
            }
        }

        float self_x = -1.f;
        float self_y = -1.f;
        if (Game::simulation.ent_exists(Game::camera_id)) {
            Entity const &camera = Game::simulation.get_ent(Game::camera_id);
            if (Game::simulation.ent_alive(camera.get_player())) {
                Entity const &me = Game::simulation.get_ent(camera.get_player());
                self_x = me.get_x();
                self_y = me.get_y();
            } else {
                self_x = camera.get_camera_x();
                self_y = camera.get_camera_y();
            }
        }

        auto is_teammate_at = [](float wx, float wy) -> bool {
            for (Game::PartyMember const &m : Game::party_members) {
                if (m.flags & 4) continue; // self
                if (!(m.x >= 0.f && m.y >= 0.f)) continue;
                if (std::abs(m.x - wx) < 80.f && std::abs(m.y - wy) < 80.f)
                    return true;
            }
            return false;
        };
        auto is_guildmate_at = [](float wx, float wy) -> bool {
            if (Game::guild_name.empty()) return false;
            for (Game::GuildMember const &m : Game::guild_members) {
                if (m.flags & 4) continue; // self
                if (!(m.flags & 2)) continue; // offline
                if (!(m.x >= 0.f && m.y >= 0.f)) continue;
                if (std::abs(m.x - wx) < 80.f && std::abs(m.y - wy) < 80.f)
                    return true;
            }
            return false;
        };

        // Other players (arena sync — every living flower on this map)
        float const pr_other = Game::minimap_expanded ? 4.5f : 3.8f;
        constexpr uint32_t kTeammatePink = 0xffff69b4;
        constexpr uint32_t kGuildBlue = 0xff3d9eff;
        for (uint32_t i = 0; i < MINIMAP_PLAYER_CAP; ++i) {
            float wx = Game::simulation.arena_info.minimap_x[i];
            float wy = Game::simulation.arena_info.minimap_y[i];
            if (!(wx >= 0.f && wy >= 0.f)) continue;
            if (self_x >= 0.f &&
                std::abs(wx - self_x) < 40.f && std::abs(wy - self_y) < 40.f)
                continue;
            float px = (wx - left) * sx;
            float py = (wy - top) * sy;
            uint8_t color = Game::simulation.arena_info.minimap_colors[i];
            if (color >= ColorID::kNumColors) color = 0;
            // Party pink > guild blue > default flower color
            uint32_t fill = FLOWER_COLORS[color];
            if (is_teammate_at(wx, wy))
                fill = kTeammatePink;
            else if (is_guildmate_at(wx, wy))
                fill = kGuildBlue;
            // White halo so dots stay visible on green/blue zones
            ctx.set_fill(0xeeffffff);
            ctx.begin_path();
            ctx.arc(px, py, pr_other + 1.6f);
            ctx.fill();
            ctx.set_fill(fill);
            ctx.set_stroke(0xff222222);
            ctx.set_line_width(1.2f);
            ctx.begin_path();
            ctx.arc(px, py, pr_other);
            ctx.fill();
            ctx.stroke();
            // Strangers / guild: names from arena sync. Party names drawn below
            // from party_members (roster is authoritative; arena names can miss teammates).
            // Fill-only names (strokeText per dot tanks Canvas2D under Hold-G).
            if (show_names && !is_teammate_at(wx, wy)) {
                std::string const &nm = Game::simulation.arena_info.minimap_names[i];
                if (!nm.empty()) {
                    RenderContext label(&ctx);
                    ctx.translate(px, py - pr_other - 8.f);
                    ctx.draw_text(nm.c_str(), {
                        .fill = 0xffffffff,
                        .stroke = 0,
                        .size = Game::minimap_expanded ? 11.f : 9.5f,
                        .stroke_scale = 0
                    });
                }
            }
        }

        // Party roster: Hold-G names from party_members (authoritative).
        // Skip members with no live/corpse pos (x<0) so leavers cannot ghost.
        auto has_arena_dot_at = [](float wx, float wy) -> bool {
            for (uint32_t i = 0; i < MINIMAP_PLAYER_CAP; ++i) {
                float ax = Game::simulation.arena_info.minimap_x[i];
                float ay = Game::simulation.arena_info.minimap_y[i];
                if (!(ax >= 0.f && ay >= 0.f)) continue;
                if (std::abs(ax - wx) < 80.f && std::abs(ay - wy) < 80.f)
                    return true;
            }
            return false;
        };
        float const pr_party = Game::minimap_expanded ? 4.5f : 3.8f;
        float const pr_down = Game::minimap_expanded ? 4.0f : 3.4f;
        float const name_sz = Game::minimap_expanded ? 11.f : 9.5f;
        for (Game::PartyMember const &m : Game::party_members) {
            if (m.flags & 4) continue; // self
            if (!(m.x >= 0.f && m.y >= 0.f)) continue;
            float px = (m.x - left) * sx;
            float py = (m.y - top) * sy;
            uint8_t const is_down = (m.flags & 2) ? 1 : 0;
            if (is_down || !has_arena_dot_at(m.x, m.y)) {
                float pr = is_down ? pr_down : pr_party;
                ctx.set_fill(is_down ? 0x88ffffff : 0xeeffffff);
                ctx.begin_path();
                ctx.arc(px, py, pr + (is_down ? 1.4f : 1.6f));
                ctx.fill();
                ctx.set_fill(is_down ? 0xccff69b4 : kTeammatePink);
                ctx.set_stroke(is_down ? 0xff882244 : 0xff222222);
                ctx.set_line_width(is_down ? 1.1f : 1.2f);
                ctx.begin_path();
                ctx.arc(px, py, pr);
                ctx.fill();
                ctx.stroke();
            }
            if (show_names && !m.name.empty()) {
                RenderContext label(&ctx);
                ctx.translate(px, py - pr_party - 8.f);
                ctx.draw_text(m.name.c_str(), {
                    .fill = is_down ? 0xffffb6d9u : 0xffffe0f0u,
                    .stroke = 0,
                    .size = name_sz,
                    .stroke_scale = 0
                });
            }
        }

        // FOV box + self (gold, on top)
        if (Game::simulation.ent_exists(Game::camera_id)) {
            Entity const &camera = Game::simulation.get_ent(Game::camera_id);
            float scale = 1.f / (2.f * camera.get_fov() * Game::camera_zoom * (float)Ui::scale);
            float fov_w = (float)Ui::window_width * scale;
            float fov_h = (float)Ui::window_height * scale;
            float fl = (camera.get_camera_x() - fov_w * 0.5f - left) * sx;
            float ft = (camera.get_camera_y() - fov_h * 0.5f - top) * sy;
            ctx.set_stroke(0xccffffff);
            ctx.set_line_width(1.2f);
            ctx.begin_path();
            ctx.rect(fl, ft, fov_w * sx, fov_h * sy);
            ctx.stroke();

            if (self_x >= 0.f) {
                float px = (self_x - left) * sx;
                float py = (self_y - top) * sy;
                float pr = Game::minimap_expanded ? 5.2f : 4.4f;
                ctx.set_fill(0xffffffff);
                ctx.begin_path();
                ctx.arc(px, py, pr + 1.8f);
                ctx.fill();
                ctx.set_fill(0xffffe763);
                ctx.set_stroke(0xffccb84a);
                ctx.set_line_width(1.5f);
                ctx.begin_path();
                ctx.arc(px, py, pr);
                ctx.fill();
                ctx.stroke();
            }
        }
    }
}

static Element *make_online_roster_list() {
    std::vector<Element *> rows;
    rows.push_back(new Ui::DynamicText(12, [](){
        return I18n::use_chinese
            ? std::format("在线 {} 人", Game::online_roster.size())
            : std::format("Online {}", Game::online_roster.size());
    }, { .fill = 0xffe8dcc8, .h_justify = Style::Left }));
    // Cap rows high enough for a full server; ScrollContainer clips.
    for (uint8_t i = 0; i < 48; ++i) {
        rows.push_back(new Ui::DynamicText(11, [i](){
            if (i >= Game::online_roster.size()) return std::string{};
            Game::OnlineRosterEntry const &e = Game::online_roster[i];
            char const *map = I18n::map_name(e.map_id);
            if (e.guild.empty())
                return std::format("{}  Lv{}  {}", e.name, e.level, map);
            return std::format("{}  Lv{}  {}  [{}]", e.name, e.level, map, e.guild);
        }, {
            .fill = 0xffcfc3b0,
            .h_justify = Style::Left,
            .should_render = [i](){ return i < Game::online_roster.size(); }
        }));
    }
    return new Ui::VContainer(rows, 4, 2, { .no_animation = 1 });
}

// Roster height fills from under the minimap down toward bottom debug strip.
class RosterScroll final : public Ui::ScrollContainer {
public:
    RosterScroll(Element *content, float initial_h)
        : Ui::ScrollContainer(content, initial_h) {
        always_capture_wheel = 1;
    }
    void refactor() override {
        float view_h = (float) (Ui::window_height / Ui::scale);
        float map_w = Game::minimap_expanded ? 420.f : 280.f;
        float map_h = map_w * (float) ARENA_HEIGHT / (float) ARENA_WIDTH;
        // Under map → above bottom-right debug strip (~120).
        float avail = view_h - 10.f - map_h - 28.f - 8.f - 130.f;
        if (avail < 180.f) avail = 180.f;
        if (avail > view_h * 0.55f) avail = view_h * 0.55f;
        height = avail;
        Ui::ScrollContainer::refactor();
    }
};

Element *Ui::make_minimap() {
    Element *map = new Ui::Minimap(280);
    Element *roster = new RosterScroll(make_online_roster_list(), 280.f);
    Element *elt = new Ui::VContainer({
        map,
        new Ui::DynamicText(11, [](){
            return Game::minimap_expanded
                ? (I18n::use_chinese ? "M 缩小" : "M shrink")
                : (I18n::use_chinese ? "M 放大" : "M enlarge");
        }, { .fill = 0xffdddddd }),
        roster
    }, 6, 4, {
        .fill = 0x00000000,
        .should_render = [](){
            return Game::should_render_game_ui()
                && (Game::alive() || Game::on_game_screen);
        },
        .h_justify = Style::Right,
        .v_justify = Style::Top
    });
    elt->x = -10;
    elt->y = 10;
    return elt;
}
