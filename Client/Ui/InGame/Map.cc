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
#include <memory>
#include <vector>

using namespace Ui;

namespace {
struct MinimapLayer {
    uint8_t map_id;
    float width;
    std::unique_ptr<Renderer> canvas;
};

std::vector<MinimapLayer> minimap_layers;

Renderer &static_minimap(uint8_t map_id, float width, float height) {
    for (MinimapLayer &layer : minimap_layers) {
        if (layer.map_id == map_id && layer.width == width)
            return *layer.canvas;
    }
    if (minimap_layers.size() >= 8) minimap_layers.clear();
    minimap_layers.push_back({map_id, width, std::make_unique<Renderer>()});
    Renderer &canvas = *minimap_layers.back().canvas;
    canvas.set_dimensions(width, height);
    canvas.reset();
    canvas.clear();
    float const sx = width / (float)ARENA_WIDTH;
    float const sy = height / (float)ARENA_HEIGHT;

    for (ZoneDefinition const &def : map_zones(map_id)) {
        canvas.set_fill(def.color);
        canvas.fill_rect(def.left * sx, def.top * sy,
                         (def.right - def.left) * sx, (def.bottom - def.top) * sy);
    }
    float oxs[TERRAIN_OUTLINE_MAX];
    float oys[TERRAIN_OUTLINE_MAX];
    for (TerrainFeature const &f : map_terrain(map_id)) {
        uint32_t n = Map::terrain_outline(f, oxs, oys, TERRAIN_OUTLINE_MAX);
        if (n < 3) continue;
        canvas.set_fill(f.color);
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

        ctx.draw_image_at(static_minimap(mid, width, height), 0.f, 0.f);

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

        // Other players (arena sync — every living flower on this map)
        float const pr_other = Game::minimap_expanded ? 4.5f : 3.8f;
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
            uint32_t fill = FLOWER_COLORS[color];
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
