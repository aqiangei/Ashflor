#include <Client/Ui/InGame/GameInfo.hh>

#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Input.hh>
#include <Client/Render/Renderer.hh>
#include <Client/Ui/Extern.hh>

#include <Helpers/Math.hh>

#include <emscripten.h>

#include <cmath>
#include <cstring>
#include <string>

using namespace Ui;

namespace Game {
uint8_t update_poster_open = 0;
}

namespace {

// Bump id when shipping a new poster so returning players get the fly-in again.
constexpr char const *kAnnounceId = "world-exploration-v1";
constexpr char const *kPosterUrl = "announcements/world-exploration-v1.svg";
constexpr char const *kStorageKey = "update_announce_seen";
constexpr double kEnterDelayMs = 600.0;
constexpr double kEnterAnimMs = 800.0;

enum class Phase : uint8_t {
    Hidden = 0,
    Waiting,
    Entering,
    Idle,
    Minimized
};

Phase g_phase = Phase::Hidden;
uint8_t g_seen = 0;
uint8_t g_alive_prev = 0;
uint8_t g_dom_ready = 0;
uint8_t g_reduced_motion = 0;
double g_phase_at = 0.0;
float g_press = 1.f;

char const *bubble_zh() { return "新版本！"; }
char const *bubble_en() { return "Update!"; }

uint8_t load_seen() {
    return (uint8_t) EM_ASM_INT({
        try {
            var k = UTF8ToString($0);
            var id = UTF8ToString($1);
            return window.localStorage[k] === id ? 1 : 0;
        } catch (e) { return 0; }
    }, kStorageKey, kAnnounceId);
}

void save_seen() {
    EM_ASM({
        try {
            window.localStorage[UTF8ToString($0)] = UTF8ToString($1);
        } catch (e) {}
    }, kStorageKey, kAnnounceId);
    g_seen = 1;
}

uint8_t prefers_reduced_motion() {
    return (uint8_t) EM_ASM_INT({
        try {
            return window.matchMedia &&
                window.matchMedia('(prefers-reduced-motion: reduce)').matches ? 1 : 0;
        } catch (e) { return 0; }
    });
}

void ensure_dom() {
    if (g_dom_ready) return;
    g_dom_ready = 1;
    EM_ASM({
        if (document.getElementById('update-poster-overlay')) return;
        var ov = document.createElement('div');
        ov.id = 'update-poster-overlay';
        ov.setAttribute('aria-hidden', 'true');
        ov.innerHTML =
            '<div class="update-poster-panel" role="dialog" aria-modal="true">' +
            '  <button type="button" class="update-poster-close" aria-label="Close">×</button>' +
            '  <div class="update-poster-scroll">' +
            '    <img class="update-poster-img" alt=""/>' +
            '    <div class="update-poster-fallback" hidden>' +
            '      <div class="update-poster-brand">ASHFLOR</div>' +
            '      <div class="update-poster-title"></div>' +
            '      <div class="update-poster-sub"></div>' +
            '      <ul class="update-poster-list"></ul>' +
            '    </div>' +
            '    <div class="update-poster-error" hidden></div>' +
            '  </div>' +
            '</div>';
        document.body.appendChild(ov);

        var close = function () {
            try {
                if (typeof Module._ashflor_update_poster_close === 'function')
                    Module._ashflor_update_poster_close();
            } catch (e) {}
        };
        ov.addEventListener('click', function (e) {
            if (e.target === ov) close();
        });
        ov.querySelector('.update-poster-close').addEventListener('click', function (e) {
            e.stopPropagation();
            close();
        });
        var img = ov.querySelector('.update-poster-img');
        img.addEventListener('error', function () {
            img.hidden = true;
            ov.querySelector('.update-poster-fallback').hidden = false;
            ov.querySelector('.update-poster-error').hidden = true;
        });
        img.addEventListener('load', function () {
            img.hidden = false;
            ov.querySelector('.update-poster-fallback').hidden = true;
            ov.querySelector('.update-poster-error').hidden = true;
        });
    });
}

void sync_poster_copy() {
    // Keep all localized copy in C++ strings — EM_ASM single-quotes choke on UTF-8.
    char const *title = I18n::use_chinese ? "八域更生" : "Eight Realms Renewed";
    char const *sub = I18n::use_chinese ? "世界不再空旷" : "The world is no longer empty";
    char const *alt = I18n::use_chinese ? "八域更生更新海报" : "Update poster";
    char const *err = I18n::use_chinese
        ? "海报加载失败"
        : "Poster failed to load";
    char const *items = I18n::use_chinese
        ? "八域探索站点与路边宝箱\n跟随 NPC 与引路偶遇\n隐藏触碰 · 限时 · 伏击环\n睡觉怪变体与环境粒子\n每域 12 槽命名事件"
        : "Explore sites & roadside chests\nFollow / guide NPC encounters\nHidden touch · timed · ambush\nSleeping mobs & biome motes\n12 named event slots per realm";
    EM_ASM({
        var ov = document.getElementById('update-poster-overlay');
        if (!ov) return;
        ov.querySelector('.update-poster-title').textContent = UTF8ToString($0);
        ov.querySelector('.update-poster-sub').textContent = UTF8ToString($1);
        ov.querySelector('.update-poster-img').alt = UTF8ToString($2);
        ov.querySelector('.update-poster-error').textContent = UTF8ToString($3);
        var list = ov.querySelector('.update-poster-list');
        while (list.firstChild) list.removeChild(list.firstChild);
        var parts = UTF8ToString($4).split('\n');
        for (var i = 0; i < parts.length; ++i) {
            if (!parts[i]) continue;
            var li = document.createElement('li');
            li.textContent = parts[i];
            list.appendChild(li);
        }
    }, title, sub, alt, err, items);
}

void show_poster_dom() {
    ensure_dom();
    sync_poster_copy();
    EM_ASM({
        var ov = document.getElementById('update-poster-overlay');
        if (!ov) return;
        var img = ov.querySelector('.update-poster-img');
        var url = UTF8ToString($0);
        ov.querySelector('.update-poster-fallback').hidden = true;
        ov.querySelector('.update-poster-error').hidden = true;
        img.hidden = false;
        // Bust only path; browser may still cache by URL.
        if (img.getAttribute('data-src') !== url) {
            img.setAttribute('data-src', url);
            img.src = url;
        }
        ov.classList.add('show');
        ov.setAttribute('aria-hidden', 'false');
        document.body.classList.add('update-poster-open');
    }, kPosterUrl);
}

void hide_poster_dom() {
    EM_ASM({
        var ov = document.getElementById('update-poster-overlay');
        if (!ov) return;
        ov.classList.remove('show');
        ov.setAttribute('aria-hidden', 'true');
        document.body.classList.remove('update-poster-open');
    });
}

void open_poster() {
    if (Game::update_poster_open) return;
    Game::update_poster_open = 1;
    if (!g_seen) save_seen();
    show_poster_dom();
}

void close_poster_internal() {
    if (!Game::update_poster_open) return;
    Game::update_poster_open = 0;
    hide_poster_dom();
    if (g_phase == Phase::Idle || g_phase == Phase::Entering)
        g_phase = Phase::Minimized;
}

void begin_session_show() {
    g_reduced_motion = prefers_reduced_motion();
    g_phase_at = Game::timestamp;
    if (g_seen) {
        g_phase = Phase::Minimized;
    } else if (g_reduced_motion) {
        g_phase = Phase::Idle;
    } else {
        g_phase = Phase::Waiting;
    }
}

void tick_lifecycle() {
    uint8_t alive = Game::alive() && Game::should_render_game_ui();
    if (alive && !g_alive_prev)
        begin_session_show();
    if (!alive) {
        if (Game::update_poster_open)
            close_poster_internal();
        g_phase = Phase::Hidden;
    }
    g_alive_prev = alive;

    if (g_phase == Phase::Waiting) {
        if (Game::timestamp - g_phase_at >= kEnterDelayMs) {
            g_phase = Phase::Entering;
            g_phase_at = Game::timestamp;
        }
    } else if (g_phase == Phase::Entering) {
        double t = Game::timestamp - g_phase_at;
        if (t >= kEnterAnimMs || g_reduced_motion)
            g_phase = Phase::Idle;
    }
}

float ease_out_cubic(float t) {
    float u = 1.f - t;
    return 1.f - u * u * u;
}

class UpdateAnnounceBubble final : public Element {
public:
    UpdateAnnounceBubble()
        : Element(156, 72, {
            .should_render = [](){
                // Include Waiting so animate can advance the enter delay.
                return Game::should_render_game_ui()
                    && Game::alive()
                    && g_phase != Phase::Hidden;
            },
            .h_justify = Style::Right,
            .v_justify = Style::Top
        }) {
        style.animate = [&](Element *elt, Renderer &ctx){
            tick_lifecycle();
            // Dock below minimap / roster; nudge down when map is enlarged.
            float dock_y = Game::minimap_expanded ? 455.f : 340.f;
            if (Input::mobile_controls())
                dock_y = Game::minimap_expanded ? 430.f : 320.f;
            elt->y = dock_y;
            elt->x = -14.f;

            uint8_t mini = g_phase == Phase::Minimized;
            float target_w = mini ? 52.f : (Input::mobile_controls() ? 118.f : 156.f);
            float target_h = mini ? 52.f : (Input::mobile_controls() ? 58.f : 72.f);
            width = lerp(width, target_w, Ui::lerp_amount * 2.4f);
            height = lerp(height, target_h, Ui::lerp_amount * 2.4f);

            float enter = 1.f;
            if (g_phase == Phase::Entering) {
                float t = (float) ((Game::timestamp - g_phase_at) / kEnterAnimMs);
                enter = ease_out_cubic(fclamp(t, 0.f, 1.f));
            }
            float bob = 0.f;
            if (g_phase == Phase::Idle && !g_reduced_motion) {
                bob = sinf((float) (Game::timestamp * 0.0024)) * 4.f;
            }
            float hover = (focus_state == kMouseHover || focus_state == kMouseMove) ? 1.06f : 1.f;
            if (focus_state == kMouseDown) hover = 0.94f;
            g_press = lerp(g_press, hover, Ui::lerp_amount * 3.f);

            if (g_phase == Phase::Waiting) {
                ctx.set_global_alpha(0.f);
                return;
            }
            ctx.set_global_alpha((float) elt->animation * (0.55f + 0.45f * enter));
            ctx.translate((1.f - enter) * 120.f, bob);
            ctx.scale(g_press);
        };
    }

    void on_render(Renderer &ctx) override {
        if (g_phase == Phase::Waiting) return;
        float hw = width / 2.f;
        float hh = height / 2.f;
        uint8_t mini = g_phase == Phase::Minimized;

        // Soft plate (warmer edge so it pops on busy HUDs)
        ctx.set_fill(0x66201810);
        ctx.begin_path();
        ctx.round_rect(-hw + 2.f, -hh + 3.f, width, height, mini ? 18.f : 16.f);
        ctx.fill();
        ctx.set_fill(0xf5fff0d0);
        ctx.begin_path();
        ctx.round_rect(-hw, -hh, width, height, mini ? 18.f : 16.f);
        ctx.fill();
        ctx.set_stroke(0xaaffe08a);
        ctx.set_line_width(2.f);
        ctx.begin_path();
        ctx.round_rect(-hw, -hh, width, height, mini ? 18.f : 16.f);
        ctx.stroke();

        // Sprite orb — bright halo + orbiting motes (petal-like)
        float sx = mini ? 0.f : -hw + 28.f;
        float sy = mini ? 0.f : -2.f;
        float t = (float) (Game::timestamp * 0.0035);
        float pulse = 1.f + 0.08f * sinf(t * 2.2f);
        ctx.set_fill(0x55ffe08a);
        ctx.begin_path();
        ctx.arc(sx, sy, (mini ? 22.f : 26.f) * pulse);
        ctx.fill();
        ctx.set_fill(0x88fff6c8);
        ctx.begin_path();
        ctx.arc(sx, sy, (mini ? 15.f : 17.f) * pulse);
        ctx.fill();
        if (!g_reduced_motion) {
            for (int i = 0; i < 8; ++i) {
                float a = t + i * 0.7854f;
                float r = (mini ? 18.f : 22.f) + 3.f * sinf(t + i);
                ctx.set_fill((i & 1) ? 0xccffffff : 0xccffe08a);
                ctx.begin_path();
                ctx.arc(sx + cosf(a) * r, sy + sinf(a) * r * 0.72f,
                        (i & 1) ? 2.8f : 2.1f);
                ctx.fill();
            }
        }
        ctx.set_fill(0xfffff4a8);
        ctx.begin_path();
        ctx.arc(sx, sy, mini ? 12.f : 13.f);
        ctx.fill();
        ctx.set_fill(0xffffffff);
        ctx.begin_path();
        ctx.arc(sx - 3.5f, sy - 3.5f, mini ? 4.5f : 5.f);
        ctx.fill();
        // Tiny wings
        ctx.set_fill(0xd0fff8ec);
        ctx.begin_path();
        ctx.ellipse(sx - 15.f, sy - 2.f, 8.f, 4.5f);
        ctx.fill();
        ctx.begin_path();
        ctx.ellipse(sx + 15.f, sy - 2.f, 8.f, 4.5f);
        ctx.fill();

        if (!mini) {
            char const *label = I18n::use_chinese ? bubble_zh() : bubble_en();
            ctx.translate(18.f, 1.f);
            ctx.draw_text(label, {
                .fill = 0xff5c3d0a,
                .stroke = 0xfff8eedc,
                .size = Input::mobile_controls() ? 14.f : 16.f,
                .stroke_scale = 0.1f
            });
            ctx.translate(-18.f, -1.f);
        }
    }

    void on_event(uint8_t event) override {
        if (event == kClick && g_phase != Phase::Waiting && g_phase != Phase::Hidden)
            open_poster();
    }

    void poll_events(ScreenEvent const &event) override {
        if (g_phase == Phase::Waiting || g_phase == Phase::Hidden) return;
        Element::poll_events(event);
    }
};

} // namespace

extern "C" void EMSCRIPTEN_KEEPALIVE ashflor_update_poster_close() {
    close_poster_internal();
}

namespace Game {
void close_update_poster() {
    close_poster_internal();
}

uint8_t update_poster_try_close() {
    if (!update_poster_open) return 0;
    close_poster_internal();
    return 1;
}
}

Element *Ui::make_update_announce_bubble() {
    g_seen = load_seen();
    ensure_dom();
    Element *elt = new UpdateAnnounceBubble();
    elt->x = -14;
    elt->y = 340;
    return elt;
}
