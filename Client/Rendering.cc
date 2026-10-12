#include <Client/Game.hh>

#include <Client/Input.hh>
#include <Client/Particle.hh>

#include <Client/Ui/Extern.hh>
#include <Client/Render/RenderEntity.hh>

#include <Helpers/Vector.hh>

#include <Shared/Map.hh>
#include <Shared/StaticData.hh>

#include <cmath>

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
    renderer.scale(Ui::scale * camera.get_fov());
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
        renderer.set_fill(0xff987d72);
        renderer.fill_rect(0,0,renderer.width,renderer.height);
        renderer.set_fill(alpha);
        renderer.fill_rect(0,0,renderer.width,renderer.height);
    }
    {
        RenderContext context(&renderer);
        uint8_t mid = Game::simulation.arena_info.map_id;
        if (mid >= MapID::kNumMaps) mid = MapID::kGarden;
        MapZones const &zones = map_zones(mid);
        for (ZoneDefinition const &def : zones) {
            renderer.set_fill(def.color);
            renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
            if (Map::difficulty_at_level(score_to_level(Game::score)) > def.difficulty) {
                renderer.set_fill(0x40000000);
                renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
            }
        }

        // Cloth weave on ground (visible tiles around camera)
        float scale = 1 / (2 * camera.get_fov() * Ui::scale);
        float leftX = camera.get_camera_x() - renderer.width * scale;
        float rightX = camera.get_camera_x() + renderer.width * scale;
        float topY = camera.get_camera_y() - renderer.height * scale;
        float bottomY = camera.get_camera_y() + renderer.height * scale;
        {
            float const weave = 64.f;
            renderer.set_stroke(MapTheme::WEAVE_DARK);
            renderer.set_line_width(1.2f);
            renderer.begin_path();
            float x0 = floorf(leftX / weave) * weave;
            float y0 = floorf(topY / weave) * weave;
            for (float x = x0; x < rightX + weave; x += weave) {
                renderer.move_to(x, topY);
                renderer.line_to(x + (bottomY - topY), bottomY);
            }
            for (float y = y0; y < bottomY + weave; y += weave) {
                renderer.move_to(leftX, y);
                renderer.line_to(rightX, y + (rightX - leftX) * 0.35f);
            }
            renderer.stroke();
            renderer.set_stroke(MapTheme::WEAVE_LIGHT);
            renderer.set_line_width(0.8f);
            renderer.begin_path();
            for (float x = x0 + weave * 0.5f; x < rightX + weave; x += weave) {
                renderer.move_to(x, topY);
                renderer.line_to(x + (bottomY - topY), bottomY);
            }
            renderer.stroke();
        }

        // Terrain: hills/water (solid cloth blocks) then channels (overlay)
        MapTerrain const &terrain = map_terrain(mid);
        auto draw_feature_fill = [&](TerrainFeature const &f) {
            renderer.set_fill(f.color);
            if (f.shape == TerrainShape::kEllipse) {
                renderer.begin_path();
                renderer.ellipse(f.left, f.top, f.right, f.bottom);
                renderer.fill();
            } else {
                renderer.fill_rect(f.left, f.top, f.right - f.left, f.bottom - f.top);
            }
        };
        for (TerrainFeature const &f : terrain) {
            if (f.kind == TerrainKind::kChannel) continue;
            draw_feature_fill(f);
            // subtle weave highlight on hills/water
            renderer.set_stroke(MapTheme::WEAVE_LIGHT);
            renderer.set_line_width(2.f);
            if (f.shape == TerrainShape::kEllipse) {
                renderer.begin_path();
                renderer.ellipse(f.left, f.top, f.right * 0.92f, f.bottom * 0.92f);
                renderer.stroke();
            } else {
                renderer.stroke_rect(f.left + 8, f.top + 8, f.right - f.left - 16, f.bottom - f.top - 16);
            }
        }
        for (TerrainFeature const &f : terrain) {
            if (f.kind != TerrainKind::kChannel) continue;
            draw_feature_fill(f);
            // Channel speckles (leaf / sand / coral feel)
            renderer.set_fill(MapTheme::WEAVE_LIGHT);
            float x0 = (f.shape == TerrainShape::kEllipse) ? (f.left - f.right) : f.left;
            float y0 = (f.shape == TerrainShape::kEllipse) ? (f.top - f.bottom) : f.top;
            float x1 = (f.shape == TerrainShape::kEllipse) ? (f.left + f.right) : f.right;
            float y1 = (f.shape == TerrainShape::kEllipse) ? (f.top + f.bottom) : f.bottom;
            for (float px = x0; px < x1; px += 55.f) {
                for (float py = y0; py < y1; py += 48.f) {
                    float sx = px + std::fmod(px * 0.37f, 18.f);
                    float sy = py + std::fmod(py * 0.29f, 14.f);
                    if (!Map::point_in_feature(f, sx, sy)) continue;
                    renderer.begin_path();
                    renderer.ellipse(sx, sy, 7.f, 4.f);
                    renderer.fill();
                }
            }
        }

        renderer.set_stroke(alpha);
        renderer.set_line_width(0.5);
        float newLeftX = ceilf(leftX / 50) * 50;
        float newTopY = ceilf(topY / 50) * 50;
        renderer.begin_path();
        for (; newLeftX < rightX; newLeftX += 50)
        {
            renderer.move_to(newLeftX, topY);
            renderer.line_to(newLeftX, bottomY);
        }
        for (; newTopY < bottomY; newTopY += 50)
        {
            renderer.move_to(leftX, newTopY);
            renderer.line_to(rightX, newTopY);
        }
        renderer.stroke();
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

    simulation.for_each<kWeb>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle());
        renderer.scale(ent.animation);
        render_web(renderer, ent);
    });
    simulation.for_each<kDrop>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle() + (ent.animation - 1) * 3 * M_PI);
        renderer.scale(ent.animation);
        render_drop(renderer, ent);
    });
    simulation.for_each<kHealth>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        render_health(renderer, ent);
    });
    simulation.for_each<kPetal>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_petal(renderer, ent);
    });
    simulation.for_each<kMob>([](Simulation *sim, Entity const &ent){
        if (ent.get_mob_id() != MobID::kAntHole) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        if (!ent.has_component(kFlower))
            renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_mob(renderer, ent);
    });
    simulation.for_each<kMob>([](Simulation *sim, Entity const &ent){
        if (ent.get_mob_id() == MobID::kAntHole) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        if (!ent.has_component(kFlower))
            renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_mob(renderer, ent);
    });
    simulation.for_each<kFlower>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        _apply_damage_filter(renderer, ent);
        render_flower(renderer, ent);
    });
    simulation.for_each<kName>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        render_name(renderer, ent);
    });
}

void Game::render_title_screen() {
    RenderContext context(&renderer);
    renderer.reset_transform();
    renderer.set_fill(0xff1ea761);
    renderer.fill_rect(0,0,renderer.width,renderer.height);
}