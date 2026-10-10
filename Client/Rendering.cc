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
        MapZones const &zones = map_zones(Game::simulation.arena_info.map_id);
        for (ZoneDefinition const &def : zones) {
            renderer.set_fill(def.color);
            renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
            if (Map::difficulty_at_level(score_to_level(Game::score)) > def.difficulty) {
                renderer.set_fill(0x40000000);
                renderer.fill_rect(def.left, def.top, def.right - def.left, def.bottom - def.top);
            }
        }
        renderer.set_stroke(alpha);
        renderer.set_line_width(0.5);
        float scale = 1 / (2 * camera.get_fov() * Ui::scale);
        float leftX = camera.get_camera_x() - renderer.width * scale;
        float rightX = camera.get_camera_x() + renderer.width * scale;
        float topY = camera.get_camera_y() - renderer.height * scale;
        float bottomY = camera.get_camera_y() + renderer.height * scale;
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