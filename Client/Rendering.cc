#include <Client/Game.hh>

#include <Client/Input.hh>
#include <Client/MapChunkCache.hh>
#include <Client/Particle.hh>
#include <Client/ThemeMaterial.hh>

#include <Client/Ui/Extern.hh>
#include <Client/Render/RenderEntity.hh>

#include <Helpers/Vector.hh>

#include <Shared/Map.hh>
#include <Shared/StaticData.hh>
#include <Shared/StaticDefinitions.hh>
#include <Shared/Talent.hh>

#include <Helpers/Math.hh>

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void draw_animated_water(Renderer &r, uint8_t map_id,
                                float left, float top, float right, float bottom,
                                double timestamp) {
    if (map_id != MapID::kOcean && map_id != MapID::kPolar
        && map_id != MapID::kVolcano) return;
    uint32_t color = map_id == MapID::kVolcano ? 0xffffaa45
                   : map_id == MapID::kPolar ? 0xffe9f8fc : 0xffb9f3f0;
    float period = map_id == MapID::kVolcano ? 11000.f : 8000.f;
    float phase = std::fmod((float)timestamp, period) / period;
    r.set_stroke(color);
    r.set_line_width(map_id == MapID::kVolcano ? 7.f : 3.f);
    r.set_global_alpha(map_id == MapID::kVolcano ? .27f : .36f);
    r.round_line_cap();
    for (TerrainFeature const &f : map_terrain(map_id)) {
        if (f.kind != TerrainKind::kWater) continue;
        if (f.shape != TerrainShape::kRiver && f.shape != TerrainShape::kSpiral) continue;
        for (int i = 0; i < 13; ++i) {
            float t = std::fmod((float)i/13.f + phase, 1.f);
            float x, y;
            if (f.shape == TerrainShape::kSpiral) Map::spiral_center_at(f, t, x, y);
            else Map::river_center_at(f, t, x, y);
            if (x < left-80.f || x > right+80.f || y < top-80.f || y > bottom+80.f)
                continue;
            r.begin_path();
            r.move_to(x-32.f, y+2.f);
            r.bcurve_to(x-12.f, y-6.f, x+12.f, y+8.f, x+36.f, y);
            r.stroke();
        }
    }
    r.set_global_alpha(1.f);
}

void _apply_damage_filter(Renderer &ctx, Entity const &ent) {
    ctx.set_global_alpha(1 - ent.deletion_animation);
    ctx.scale(1 + 0.5 * ent.deletion_animation);
    if (ent.damage_flash > 0.66)
        ctx.add_color_filter(0xffffffff, ent.damage_flash);
    else if (ent.damage_flash > 0.1)
        ctx.add_color_filter(0xffff1200, ent.damage_flash * 1.5);
}

static float screen_shake_radius = 0;

void Game::render_game() {
    RenderContext context(&renderer);
    DEBUG_ONLY(assert(simulation.ent_exists(camera_id));)
    Entity const &camera = simulation.get_ent(camera_id);
    renderer.translate(renderer.width / 2, renderer.height / 2);
    renderer.scale(Ui::scale * camera.get_fov() * Game::camera_zoom);
    renderer.translate(-camera.get_camera_x(), -camera.get_camera_y());
    if (alive()) {
        Entity const &player = simulation.get_ent(player_id);
        if (Game::timestamp - player.last_damaged_time < 150) {
            screen_shake_radius = lerp(screen_shake_radius, 5, Ui::lerp_amount);
        } else {
            screen_shake_radius = lerp(screen_shake_radius, 0, 2 * Ui::lerp_amount);
        }
        if (screen_shake_radius > 0.01) {
            Vector rand = Vector::rand(std::sqrt(frand()) * screen_shake_radius);
            renderer.translate(rand.x, rand.y);
        }
    }
    uint32_t alpha = (uint32_t)(camera.get_fov() * 255 * 0.2) << 24;
    {
        RenderContext context(&renderer);
        renderer.reset_transform();
        // Stable void color outside the arena (no FOV-tinted wash — that pulsed).
        renderer.set_fill(0xff2a2624);
        renderer.fill_rect(0, 0, renderer.width, renderer.height);
    }
    {
        RenderContext context(&renderer);
        uint8_t mid = Game::simulation.arena_info.map_id;
        if (mid >= MapID::kNumMaps) mid = MapID::kGarden;
        float scale = 1.f / (2.f * camera.get_fov() * Game::camera_zoom * Ui::scale);
        float leftX = camera.get_camera_x() - renderer.width * scale;
        float rightX = camera.get_camera_x() + renderer.width * scale;
        float topY = camera.get_camera_y() - renderer.height * scale;
        float bottomY = camera.get_camera_y() + renderer.height * scale;

        // Clip all ground / water / grid to the arena so edge-chunk padding and
        // bake hole-fills cannot paint (or flash) into the void.
        {
            RenderContext arena_clip(&renderer);
            renderer.begin_path();
            renderer.rect(0.f, 0.f, (float)ARENA_WIDTH, (float)ARENA_HEIGHT);
            renderer.clip();

            MapChunkCache::draw(renderer, mid,
                                Map::difficulty_at_level(score_to_level(Game::score)),
                                leftX, topY, rightX, bottomY, Input::is_mobile);
            draw_animated_water(renderer, mid, leftX, topY, rightX, bottomY,
                                Game::timestamp);

            float grid_l = leftX < 0.f ? 0.f : leftX;
            float grid_t = topY < 0.f ? 0.f : topY;
            float grid_r = rightX > (float)ARENA_WIDTH ? (float)ARENA_WIDTH : rightX;
            float grid_b = bottomY > (float)ARENA_HEIGHT ? (float)ARENA_HEIGHT : bottomY;
            if (grid_r > grid_l && grid_b > grid_t) {
                renderer.set_stroke(alpha);
                renderer.set_line_width(0.5f);
                float newLeftX = ceilf(grid_l / 50.f) * 50.f;
                float newTopY = ceilf(grid_t / 50.f) * 50.f;
                renderer.begin_path();
                for (; newLeftX < grid_r; newLeftX += 50.f) {
                    renderer.move_to(newLeftX, grid_t);
                    renderer.line_to(newLeftX, grid_b);
                }
                for (; newTopY < grid_b; newTopY += 50.f) {
                    renderer.move_to(grid_l, newTopY);
                    renderer.line_to(grid_r, newTopY);
                }
                renderer.stroke();
            }
        }
    }
    // Spin-rate hint ring around self while adjusting (Q/E hold or drag wheel)
    if (alive() && Game::rot_hint_opacity > 0.02f) {
        Entity const &player = simulation.get_ent(player_id);
        float t = Game::rot_speed_f / (float) Talent::kRotSettingMax;
        float alpha = fclamp(Game::rot_hint_opacity, 0.f, 1.f);
        float pr = player.get_radius() + 28.f + 10.f * t;
        float spin = (float) (Game::timestamp * 0.0035 * (0.35 + t * 1.85));
        RenderContext hint_ctx(&renderer);
        renderer.set_global_alpha(alpha * 0.85f);
        renderer.set_stroke(0xff66c2ff);
        renderer.set_line_width(3.5f);
        renderer.begin_path();
        renderer.partial_arc(player.get_x(), player.get_y(), pr,
            -M_PI / 2, -M_PI / 2 + 2 * M_PI * t, 0);
        renderer.stroke();
        renderer.set_stroke(0xffffffff);
        renderer.set_line_width(2.5f);
        renderer.round_line_cap();
        renderer.begin_path();
        renderer.move_to(
            player.get_x() + cosf(spin) * (pr - 10.f),
            player.get_y() + sinf(spin) * (pr - 10.f));
        renderer.line_to(
            player.get_x() + cosf(spin) * (pr + 4.f),
            player.get_y() + sinf(spin) * (pr + 4.f));
        renderer.stroke();
        renderer.set_global_alpha(1.f);
    }
    // Antenna / FOV helper — thin ray toward cursor (default on)
    if (alive() && Input::movement_helper && !Input::is_mobile) {
        Entity const &player = simulation.get_ent(player_id);
        float mx = (Input::mouse_x - renderer.width / 2) / Ui::scale;
        float my = (Input::mouse_y - renderer.height / 2) / Ui::scale;
        Vector tip{mx, my};
        float dist = tip.magnitude();
        float start_r = player.get_radius() + 8;
        if (dist > start_r + 4) {
            RenderContext context(&renderer);
            renderer.reset_transform();
            renderer.translate(renderer.width / 2, renderer.height / 2);
            renderer.scale(Ui::scale);
            Vector start = tip;
            start.set_magnitude(start_r);
            float ang = tip.angle();
            float c = cosf(ang), s = sinf(ang);
            auto draw_antenna = [&](uint32_t color, float lw, float head) {
                renderer.set_stroke(color);
                renderer.set_line_width(lw);
                renderer.round_line_cap();
                renderer.round_line_join();
                renderer.begin_path();
                renderer.move_to(start.x, start.y);
                renderer.line_to(mx, my);
                float bx = mx - c * head, by = my - s * head;
                float px = -s * head * 0.55f, py = c * head * 0.55f;
                renderer.move_to(mx, my);
                renderer.line_to(bx + px, by + py);
                renderer.move_to(mx, my);
                renderer.line_to(bx - px, by - py);
                renderer.stroke();
            };
            draw_antenna(0x99000000, 3.5f, 16);
            draw_antenna(0xbbcccccc, 2.2f, 14);
        }
    }

    Particle::tick_game(renderer, Ui::dt);

    // Rendering is independent of simulation: skip entities outside the camera
    // while retaining a margin for large sprites and labels.
    float const world_scale = Ui::scale * camera.get_fov() * Game::camera_zoom;
    float const view_half_w = renderer.width / (2 * world_scale);
    float const view_half_h = renderer.height / (2 * world_scale);
    auto visible = [&](Entity const &ent) {
        float const margin = std::max(200.f, (float)ent.get_radius() * 2.f);
        return std::fabs((float)ent.get_x() - (float)camera.get_camera_x()) <= view_half_w + margin
            && std::fabs((float)ent.get_y() - (float)camera.get_camera_y()) <= view_half_h + margin;
    };

    simulation.for_each<kWeb>([&](Simulation *sim, Entity const &ent){
        if (!visible(ent)) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle());
        renderer.scale(ent.animation);
        render_web(renderer, ent);
    });
    simulation.for_each<kDrop>([&](Simulation *sim, Entity const &ent){
        if (!visible(ent)) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle() + (ent.animation - 1) * 3 * M_PI);
        renderer.scale(ent.animation);
        render_drop(renderer, ent);
    });
    simulation.for_each<kPetal>([&](Simulation *sim, Entity const &ent){
        if (!visible(ent)) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_petal(renderer, ent);
    });
    simulation.for_each<kMob>([&](Simulation *sim, Entity const &ent){
        if (!visible(ent)) return;
        if (ent.get_mob_id() != MobID::kAntHole) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        if (!ent.has_component(kFlower))
            renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_mob(renderer, ent);
    });
    simulation.for_each<kMob>([&](Simulation *sim, Entity const &ent){
        if (!visible(ent)) return;
        if (ent.get_mob_id() == MobID::kAntHole) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        if (!ent.has_component(kFlower))
            renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_mob(renderer, ent);
    });
    simulation.for_each<kFlower>([&](Simulation *sim, Entity const &ent){
        if (!visible(ent)) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        _apply_damage_filter(renderer, ent);
        render_flower(renderer, ent);
    });
    // HP / name / rarity AFTER bodies — was drawn under mobs so large rarities
    // painted over the chrome (esp. Unique+ radius scaling).
    simulation.for_each<kHealth>([&](Simulation *sim, Entity const &ent){
        if (!visible(ent)) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        render_health(renderer, ent);
    });
    simulation.for_each<kName>([&](Simulation *sim, Entity const &ent){
        if (!visible(ent)) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        render_name(renderer, ent);
    });
}

void Game::render_title_screen() {
    RenderContext context(&renderer);
    uint8_t mid = Game::selected_map;
    if (mid >= MapID::kNumMaps) mid = MapID::kGarden;
    TerrainArt::ThemeMaterial const &tm = TerrainArt::theme(mid);

    renderer.reset_transform();
    renderer.set_fill(tm.ground);
    renderer.fill_rect(0, 0, renderer.width, renderer.height);

    // Slow pan over spawn plaza so the backdrop reads as that map's ThemeMaterial.
    // Half the in-game-ish FOV (0.62 → 0.31) so title preview shows ~2× the world.
    float const fov = 0.31f;
    float const t = (float)(Game::timestamp * 0.00018);
    float const cam_x = MAP_SPAWN_X + std::sin(t) * 480.f;
    float const cam_y = MAP_SPAWN_Y + std::cos(t * 0.81f) * 360.f;
    {
        RenderContext world(&renderer);
        renderer.translate(renderer.width / 2, renderer.height / 2);
        renderer.scale(Ui::scale * fov);
        renderer.translate(-cam_x, -cam_y);
        float const scale = 1.f / (2.f * fov * Ui::scale);
        float const leftX = cam_x - renderer.width * scale;
        float const rightX = cam_x + renderer.width * scale;
        float const topY = cam_y - renderer.height * scale;
        float const bottomY = cam_y + renderer.height * scale;
        MapChunkCache::draw(renderer, mid, 0, leftX, topY, rightX, bottomY, Input::is_mobile);
    }
}
