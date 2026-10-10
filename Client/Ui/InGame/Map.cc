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

Minimap::Minimap(float w) : Element(w, w * ARENA_HEIGHT / (float)ARENA_WIDTH, {}) {}

void Minimap::refactor() {
    if (Game::minimap_expanded) {
        width = 360;
        height = width * ARENA_HEIGHT / (float)ARENA_WIDTH;
    } else {
        // Local detail view — more readable aspect than the full 10:1 strip
        width = 200;
        height = 110;
    }
}

void Minimap::on_render(Renderer &ctx) {
    ctx.set_fill(0xe0111111);
    ctx.begin_path();
    ctx.round_rect(-width / 2, -height / 2, width, height, 4);
    ctx.fill();
    ctx.set_line_width(3);
    ctx.set_stroke(0xffeeeeee);
    ctx.begin_path();
    ctx.round_rect(-width / 2, -height / 2, width, height, 4);
    ctx.stroke();

    float view_w;
    float view_h;
    float view_cx;
    float view_cy;

    if (!Game::simulation.ent_exists(Game::camera_id)) {
        view_w = (float)ARENA_WIDTH;
        view_h = (float)ARENA_HEIGHT;
        view_cx = view_w * 0.5f;
        view_cy = view_h * 0.5f;
    } else {
        Entity const &camera = Game::simulation.get_ent(Game::camera_id);
        if (Game::minimap_expanded) {
            view_w = (float)ARENA_WIDTH;
            view_h = (float)ARENA_HEIGHT;
            view_cx = view_w * 0.5f;
            view_cy = view_h * 0.5f;
        } else {
            // Zoomed local window around the player
            view_w = 7000.f;
            view_h = view_w * height / width;
            view_cx = camera.get_camera_x();
            view_cy = camera.get_camera_y();
            view_cx = fclamp(view_cx, view_w * 0.5f, ARENA_WIDTH - view_w * 0.5f);
            view_cy = fclamp(view_cy, view_h * 0.5f, ARENA_HEIGHT - view_h * 0.5f);
        }
    }

    float left = view_cx - view_w * 0.5f;
    float top = view_cy - view_h * 0.5f;
    float sx = width / view_w;
    float sy = height / view_h;

    {
        RenderContext clip(&ctx);
        ctx.begin_path();
        ctx.rect(-width / 2, -height / 2, width, height);
        ctx.clip();

        ctx.translate(-width / 2, -height / 2);
        MapZones const &zones = map_zones(Game::simulation.arena_info.map_id);
        for (uint32_t zi = 0; zi < zones.size(); ++zi) {
            ZoneDefinition const &def = zones[zi];
            float zl = (def.left - left) * sx;
            float zt = (def.top - top) * sy;
            float zw = (def.right - def.left) * sx;
            float zh = (def.bottom - def.top) * sy;
            if (zl + zw < 0 || zt + zh < 0 || zl > width || zt > height) continue;
            ctx.set_fill(def.color);
            ctx.fill_rect(zl, zt, zw, zh);
            if (Game::minimap_expanded && zh > 10) {
                ctx.translate(zl + zw / 2, zt + zh / 2);
                ctx.draw_text(I18n::zone_name(zi), { .size = std::min(zh * 0.45f, 14.f) });
                ctx.translate(-(zl + zw / 2), -(zt + zh / 2));
            }
        }

        // Other players (from arena sync)
        {
            float pr_other = Game::minimap_expanded ? 3.f : 3.5f;
            float self_x = -1.f;
            float self_y = -1.f;
            if (Game::simulation.ent_exists(Game::camera_id)) {
                Entity const &camera = Game::simulation.get_ent(Game::camera_id);
                self_x = camera.get_camera_x();
                self_y = camera.get_camera_y();
            }
            for (uint32_t i = 0; i < MINIMAP_PLAYER_CAP; ++i) {
                float wx = Game::simulation.arena_info.minimap_x[i];
                float wy = Game::simulation.arena_info.minimap_y[i];
                if (wx < 0.f || wy < 0.f) continue;
                // Skip self (drawn as gold below)
                if (self_x >= 0.f &&
                    std::abs(wx - self_x) < 40.f && std::abs(wy - self_y) < 40.f)
                    continue;
                float px = (wx - left) * sx;
                float py = (wy - top) * sy;
                if (px < -4 || py < -4 || px > width + 4 || py > height + 4) continue;
                uint8_t color = Game::simulation.arena_info.minimap_colors[i];
                if (color >= ColorID::kNumColors) color = 0;
                uint32_t fill = FLOWER_COLORS[color];
                ctx.set_fill(fill);
                ctx.set_stroke(Renderer::HSV(fill, 0.75f));
                ctx.set_line_width(1.2f);
                ctx.begin_path();
                ctx.arc(px, py, pr_other);
                ctx.fill();
                ctx.stroke();
            }
        }

        // Visible FOV rectangle + self
        if (Game::simulation.ent_exists(Game::camera_id)) {
            Entity const &camera = Game::simulation.get_ent(Game::camera_id);
            float scale = 1.f / (2.f * camera.get_fov() * (float)Ui::scale);
            float fov_w = (float)Ui::window_width * scale;
            float fov_h = (float)Ui::window_height * scale;
            float fl = (camera.get_camera_x() - fov_w * 0.5f - left) * sx;
            float ft = (camera.get_camera_y() - fov_h * 0.5f - top) * sy;
            ctx.set_stroke(0xccffffff);
            ctx.set_line_width(1.5f);
            ctx.begin_path();
            ctx.rect(fl, ft, fov_w * sx, fov_h * sy);
            ctx.stroke();

            float px = (camera.get_camera_x() - left) * sx;
            float py = (camera.get_camera_y() - top) * sy;
            float pr = Game::minimap_expanded ? 4.f : 5.f;
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

Element *Ui::make_minimap() {
    Element *map = new Ui::Minimap(200);
    Element *elt = new Ui::VContainer({
        map,
        new Ui::DynamicText(11, [](){
            return Game::minimap_expanded
                ? (I18n::use_chinese ? "M 小地图" : "M minimap")
                : (I18n::use_chinese ? "M 全图" : "M full map");
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
