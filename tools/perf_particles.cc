// Native particle lifetime regression. The renderer is inert so this checks
// admission, aging, reset, and viewport behavior without a browser or GPU.
// clang++ -std=c++20 -O3 -DCLIENTSIDE=1 -Wno-narrowing -I. \
//   -ffunction-sections -fdata-sections tools/perf_particles.cc \
//   Helpers/Math.cc Helpers/Vector.cc Shared/EntityDef.cc \
//   -Wl,--gc-sections -o /tmp/perf-particles
#include <Client/Game.hh>
#include <Client/Input.hh>
namespace Game { double timestamp = 0; uint8_t alive() { return 0; } }
namespace Input { uint8_t is_mobile = 0; }
#include <Client/Particle.cc>
#include <cstdio>

RenderContext::RenderContext() : renderer(nullptr) {}
RenderContext::RenderContext(Renderer *r) : renderer(r) {}
RenderContext::~RenderContext() {}
Renderer::Renderer() {}
Renderer::~Renderer() {}
void Renderer::set_global_alpha(float) {}
void Renderer::set_fill(uint32_t) {}
void Renderer::begin_path() {}
void Renderer::arc(float, float, float) {}
void Renderer::fill(uint8_t) {}
void Renderer::translate(float, float) {}
void Renderer::draw_text(char const *, TextArgs const) {}

static void require(bool ok, char const *message) {
    if (!ok) { std::fprintf(stderr, "FAIL %s\n", message); std::exit(1); }
}

static void advance(Renderer &renderer, double dt) {
    Game::timestamp += dt;
    Particle::tick_game(renderer, dt);
}

int main() {
    Renderer renderer;
    for (uint8_t mobile : {0, 1}) {
        Input::is_mobile = mobile;
        Game::timestamp = 0;
        Particle::reset();
        Particle::set_view(0, 0, 1000, 1000, 0);
        size_t maximum = 0;
        for (int frame = 0; frame < 18000; ++frame) { // five minutes at 60fps
            advance(renderer, 16.667);
            size_t before = Particle::game_particle_count();
            for (int petal = 0; petal < 200; ++petal)
                Particle::add_unique_particle(500, 500, 33);
            Particle::add_spirit_trail(500, 500, 1, 0);
            require(Particle::game_particle_count() - before <= (mobile ? 12u : 24u),
                    "per-frame emission budget");
            Particle::add_damage_number(EntityID(1, 0), 500, 500, 10);
            Particle::add_heal_number(500, 500, 10);
            maximum = std::max(maximum, Particle::game_particle_count());
            require(maximum <= (mobile ? 192u : 384u), "live particle cap");
            require(floating_texts.size() <= kMaxFloatingTexts, "floating text cap");
        }
        require(maximum > 0, "visible particles emitted");
        // Aging uses both delta time and the absolute clock (combat cooldowns).
        // Leave time for the final pending damage pop, its lifetime, and eviction.
        for (int i = 0; i < 3; ++i) advance(renderer, 2000);
        require(Particle::game_particle_count() == 0, "particles expire");
        require(floating_texts.empty() && dmg_buckets.empty(), "combat text expires");

        // Check culling with an empty budget so the live cap cannot mask a failure.
        Particle::add_unique_particle(10000, 10000, 33);
        Particle::add_spirit_trail(10000, 10000, 1, 0);
        Particle::add_damage_number(EntityID(2, 0), 10000, 10000, 10);
        Particle::add_heal_number(10000, 10000, 10);
        require(Particle::game_particle_count() == 0, "offscreen emitter rejected");
        require(floating_texts.empty(), "offscreen combat text rejected");
        Particle::add_unique_particle(500, 500);
        require(Particle::game_particle_count() > 0, "offscreen emitters preserve budget");
        size_t count = Particle::game_particle_count();
        Particle::set_view(0, 0, 1000, 1000, 0);
        require(count == Particle::game_particle_count(), "same-map view preserves effects");
        Particle::set_view(0, 0, 1000, 1000, 1);
        require(Particle::game_particle_count() == 0, "map change clears effects");
        require(floating_texts.empty() && dmg_buckets.empty(), "map change clears combat state");
        Particle::add_unique_particle(500, 500);
        Particle::add_heal_number(500, 500, 10);
        Particle::reset();
        require(Particle::game_particle_count() == 0 && floating_texts.empty()
                && dmg_buckets.empty(), "session reset clears effects");
        std::printf("PASS mobile=%u simulated_minutes=5 max_particles=%zu\n", mobile, maximum);
    }
    Particle::reset();
}
