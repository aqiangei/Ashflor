#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Ui/ScrollContainer.hh>
#include <Client/Game.hh>
#include <Client/Input.hh>
#include <Client/I18n.hh>
#include <Client/Ui/Extern.hh>

#include <Shared/Map.hh>
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
        ctx.draw_image_at(static_minimap(mid, width, height, show_levels), 0.f, 0.f);

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
        }

        // DOWN teammates (corpse pos) — pink, not in living minimap list
        float const pr_down = Game::minimap_expanded ? 4.0f : 3.4f;
        for (Game::PartyMember const &m : Game::party_members) {
            if (m.flags & 4) continue;
            if (!(m.flags & 2)) continue;
            if (!(m.x >= 0.f && m.y >= 0.f)) continue;
            float px = (m.x - left) * sx;
            float py = (m.y - top) * sy;
            ctx.set_fill(0x88ffffff);
            ctx.begin_path();
            ctx.arc(px, py, pr_down + 1.4f);
            ctx.fill();
            ctx.set_fill(0xccff69b4);
            ctx.set_stroke(0xff882244);
            ctx.set_line_width(1.1f);
            ctx.begin_path();
            ctx.arc(px, py, pr_down);
            ctx.fill();
            ctx.stroke();
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
    for (uint8_t i = 0; i < 12; ++i) {
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

Element *Ui::make_minimap() {
    Element *map = new Ui::Minimap(280);
    Element *roster = new Ui::ScrollContainer(make_online_roster_list(), 140);
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
        .should_render = [](){ return Game::should_render_game_ui() && Game::alive(); },
        .h_justify = Style::Right,
        .v_justify = Style::Top
    });
    elt->x = -10;
    elt->y = 10;
    return elt;
}
