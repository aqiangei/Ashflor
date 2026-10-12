#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Ui/Container.hh>
#include <Client/Ui/DynamicText.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Ui/Extern.hh>

#include <Shared/Map.hh>
#include <Client/StaticData.hh>
#include <Helpers/Math.hh>

#include <algorithm>
#include <cmath>

using namespace Ui;

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
    ctx.round_rect(-width / 2, -height / 2, width, height, 3);
    ctx.fill();
    ctx.set_line_width(2.5f);
    ctx.set_stroke(0xffeeeeee);
    ctx.begin_path();
    ctx.round_rect(-width / 2, -height / 2, width, height, 3);
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

        MapZones const &zones = map_zones(mid);
        for (uint32_t zi = 0; zi < zones.size(); ++zi) {
            ZoneDefinition const &def = zones[zi];
            float zl = (def.left - left) * sx;
            float zt = (def.top - top) * sy;
            float zw = (def.right - def.left) * sx;
            float zh = (def.bottom - def.top) * sy;
            ctx.set_fill(def.color);
            ctx.fill_rect(zl, zt, zw, zh);
        }

        MapTerrain const &terrain = map_terrain(mid);
        for (TerrainFeature const &f : terrain) {
            ctx.set_fill(f.color);
            if (f.shape == TerrainShape::kEllipse) {
                float cx = (f.left - left) * sx;
                float cy = (f.top - top) * sy;
                float rx = std::max(f.right * sx, 1.2f);
                float ry = std::max(f.bottom * sy, 1.2f);
                ctx.begin_path();
                ctx.ellipse(cx, cy, rx, ry);
                ctx.fill();
            } else {
                float zl = (f.left - left) * sx;
                float zt = (f.top - top) * sy;
                float zw = std::max((f.right - f.left) * sx, 1.f);
                float zh = std::max((f.bottom - f.top) * sy, 1.f);
                ctx.fill_rect(zl, zt, zw, zh);
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

        // All other players (arena sync — every living flower on this map)
        float const pr_other = Game::minimap_expanded ? 3.2f : 2.8f;
        for (uint32_t i = 0; i < MINIMAP_PLAYER_CAP; ++i) {
            float wx = Game::simulation.arena_info.minimap_x[i];
            float wy = Game::simulation.arena_info.minimap_y[i];
            if (wx < 0.f || wy < 0.f) continue;
            if (self_x >= 0.f &&
                std::abs(wx - self_x) < 1.f && std::abs(wy - self_y) < 1.f)
                continue;
            float px = (wx - left) * sx;
            float py = (wy - top) * sy;
            uint8_t color = Game::simulation.arena_info.minimap_colors[i];
            if (color >= ColorID::kNumColors) color = 0;
            uint32_t fill = FLOWER_COLORS[color];
            ctx.set_fill(fill);
            ctx.set_stroke(Renderer::HSV(fill, 0.7f));
            ctx.set_line_width(1.1f);
            ctx.begin_path();
            ctx.arc(px, py, pr_other);
            ctx.fill();
            ctx.stroke();
        }

        // FOV box + self (gold, on top)
        if (Game::simulation.ent_exists(Game::camera_id)) {
            Entity const &camera = Game::simulation.get_ent(Game::camera_id);
            float scale = 1.f / (2.f * camera.get_fov() * (float)Ui::scale);
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
                float pr = Game::minimap_expanded ? 4.2f : 3.6f;
                ctx.set_fill(0xffffe763);
                ctx.set_stroke(0xffccb84a);
                ctx.set_line_width(1.4f);
                ctx.begin_path();
                ctx.arc(px, py, pr);
                ctx.fill();
                ctx.stroke();
            }
        }
    }
}

Element *Ui::make_minimap() {
    Element *map = new Ui::Minimap(280);
    Element *elt = new Ui::VContainer({
        map,
        new Ui::DynamicText(11, [](){
            return Game::minimap_expanded
                ? (I18n::use_chinese ? "M 缩小" : "M shrink")
                : (I18n::use_chinese ? "M 放大" : "M enlarge");
        }, { .fill = 0xffdddddd })
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
