#include <Client/Setup.hh>

#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/Input.hh>
#include <Client/Storage.hh>
#include <Client/Ui/InGame/Loadout.hh>

#include <unordered_map>

#include <emscripten.h>

extern "C" void EMSCRIPTEN_KEEPALIVE ashflor_cancel_petal_drags() {
    Ui::cancel_petal_drags();
}

static char _get_key_from_code(std::string const &code) {
    static std::unordered_map<std::string, char> const KEYCODE_MAP = 
        {{"AltLeft", 18},{"AltRight", 18},{"ArrowDown", 40},
        {"ArrowLeft", 37},{"ArrowRight", 39},{"ArrowUp", 38},
        {"Backquote", '`'},{"Backslash", '\\'},{"Backspace", 8},
        {"BracketLeft", '['},{"BracketRight", ']'},{"CapsLock", 20},
        {"Comma", ','},{"ControlLeft", 17},{"ControlRight", 17},
        {"Delete", 46},{"Digit0", '0'},{"Digit1", '1'},{"Digit2", '2'},
        {"Digit3", '3'},{"Digit4", '4'},{"Digit5", '5'},{"Digit6", '6'},
        {"Digit7", '7'},{"Digit8", '8'},{"Digit9", '9'},{"Enter", '\r'},
        {"NumpadEnter", '\r'},
        {"Equal", '='},{"Escape", 27},{"F1", 112},{"F10", 121},
        {"F11", 122},{"F12", 123},{"F2", 113},{"F3", 114},
        {"F4", 115},{"F5", 116},{"F6", 117},{"F7", 118},{"F8", 119},
        {"F9", 120},{"Insert", 45},{"KeyA", 'A'},{"KeyB", 'B'},
        {"KeyC", 'C'},{"KeyD", 'D'},{"KeyE", 'E'},{"KeyF", 'F'},
        {"KeyG", 'G'},{"KeyH", 'H'},{"KeyI", 'I'},{"KeyJ", 'J'},
        {"KeyK", 'K'},{"KeyL", 'L'},{"KeyM", 'M'},{"KeyN", 'N'},
        {"KeyO", 'O'},{"KeyP", 'P'},{"KeyQ", 'Q'},{"KeyR", 'R'},
        {"KeyS", 'S'},{"KeyT", 'T'},{"KeyU", 'U'},{"KeyV", 'V'},
        {"KeyW", 'W'},{"KeyX", 'X'},{"KeyY", 'Y'},{"KeyZ", 'Z'},
        {"MetaLeft", 91},{"Minus", '-'},{"Period", '.'},
        {"Quote", '\''},{"Semicolon", ';'},{"ShiftLeft", '\x10'},
        {"ShiftRight", '\x10'},{"Slash", '/'},{"Space", ' '},{"Tab", 9}};
    auto place = KEYCODE_MAP.find(code);
    if (place == KEYCODE_MAP.end()) return 0;
    return place->second;
}

extern "C" {
    void mouse_event(float x, float y, uint8_t type, uint8_t button) {
        Input::mouse_x = x;
        Input::mouse_y = y;
        if (type == 0) {
            BitMath::set(Input::mouse_buttons_pressed, button);
            BitMath::set(Input::mouse_buttons_state, button);
        }
        else if (type == 2) {
            BitMath::set(Input::mouse_buttons_released, button);
            BitMath::unset(Input::mouse_buttons_state, button);
        }
    }

    void key_event(char *code, uint8_t type) {
        char button = _get_key_from_code(std::string(code));
        if (type == 0) {
            Input::keys_held.insert(button);
            Input::keys_held_this_tick.insert(button);
        }
        else if (type == 1) Input::keys_held.erase(button);
        free(code);
    }

    void touch_event(float x, float y, uint8_t type, uint32_t id) {
        if (type == 0) {
            Input::touches.insert({id, { .id = id, .x = x, .y = y, .dx = 0, .dy = 0, .saturated = 0 }});
        } else if (type == 2) {
            Input::touches.erase(id);
        } else {
            auto iter = Input::touches.find(id);
            if (iter == Input::touches.end()) return;
            Input::Touch &touch = iter->second;
            touch.dx = x - touch.x;
            touch.dy = y - touch.y;
            touch.x = x;
            touch.y = y;
        }
    }

    void wheel_event(float wheel) {
        Input::wheel_delta = wheel;
    }

    void clipboard_event(char *clipboard) {
        Input::clipboard = std::string(clipboard);
        free(clipboard);
    }

    void loop(double d, float width, float height) {
        Game::renderer.width = width;
        Game::renderer.height = height;
        Game::tick(d);
    }

}

int setup_inputs() {
    EM_ASM({
        // Cap DPR: full 3x canvas on phones makes Canvas2D feel lagged
        Module.ashflorDpr = Math.min(window.devicePixelRatio || 1, 2);
        const dpr = () => Module.ashflorDpr || Math.min(window.devicePixelRatio || 1, 2);
        window.addEventListener("keydown", (e) => {
            const typing = e.target && (e.target.tagName === "INPUT" || e.target.tagName === "TEXTAREA");
            const pass = e.code === "Escape" || e.code === "Enter" || e.code === "NumpadEnter" || e.code === "Tab";
            if (typing && !pass) return;
            if (typing && pass) e.preventDefault();
            !e.repeat && _key_event(stringToNewUTF8(e.code), 0);
        });
        window.addEventListener("keyup", (e) => {
            const typing = e.target && (e.target.tagName === "INPUT" || e.target.tagName === "TEXTAREA");
            const pass = e.code === "Escape" || e.code === "Enter" || e.code === "NumpadEnter" || e.code === "Tab";
            if (typing && !pass) return;
            !e.repeat && _key_event(stringToNewUTF8(e.code), 1);
        });
        window.addEventListener("contextmenu", (e) => { e.preventDefault(); });
        window.addEventListener("mousedown", (e) => {
            //e.preventDefault();
            const r = dpr();
            _mouse_event(e.clientX * r, e.clientY * r, 0, +!!e.button);
        });
        window.addEventListener("mousemove", (e) => {
            //e.preventDefault();
            const r = dpr();
            _mouse_event(e.clientX * r, e.clientY * r, 1, +!!e.button);
        });
        window.addEventListener("mouseup", (e) => {
            //e.preventDefault();
            const r = dpr();
            _mouse_event(e.clientX * r, e.clientY * r, 2, +!!e.button);
        });
        const touchOnField = (e) => {
            const t = e.target;
            return t && (t.tagName === "INPUT" || t.tagName === "TEXTAREA");
        };
        window.addEventListener("touchstart", (e) => {
            if (!touchOnField(e)) e.preventDefault();
            const r = dpr();
            for (const t of e.changedTouches)
                _touch_event(t.clientX * r, t.clientY * r, 0, t.identifier);
        }, { passive: false });
        window.addEventListener("touchmove", (e) => {
            if (!touchOnField(e)) e.preventDefault();
            const r = dpr();
            for (const t of e.changedTouches)
                _touch_event(t.clientX * r, t.clientY * r, 1, t.identifier);
        }, { passive: false });
        window.addEventListener("touchend", (e) => {
            if (!touchOnField(e)) e.preventDefault();
            const r = dpr();
            for (const t of e.changedTouches)
                _touch_event(t.clientX * r, t.clientY * r, 2, t.identifier);
        }, { passive: false });
        window.addEventListener("touchcancel", (e) => {
            const r = dpr();
            for (const t of e.changedTouches)
                _touch_event(t.clientX * r, t.clientY * r, 2, t.identifier);
        }, { passive: false });
        window.addEventListener("paste", (e) => {
            try {
                const clip = e.clipboardData.getData("text/plain");
                _clipboard_event(stringToNewUTF8(clip));
            } catch(e) {
            };
        }, { capture: true });
        window.addEventListener("wheel", (e) => {
            e.preventDefault();
            _wheel_event(e.deltaY);
        }, { passive: false });
        // Blur / tab-hide: abort petal drag so the next click cannot drop stale state
        window.addEventListener("blur", () => {
            if (typeof _ashflor_cancel_petal_drags === "function")
                _ashflor_cancel_petal_drags();
            else if (typeof Module !== "undefined" && Module["_ashflor_cancel_petal_drags"])
                Module["_ashflor_cancel_petal_drags"]();
        });
        document.addEventListener("visibilitychange", () => {
            if (document.visibilityState !== "hidden") return;
            if (typeof _ashflor_cancel_petal_drags === "function")
                _ashflor_cancel_petal_drags();
            else if (typeof Module !== "undefined" && Module["_ashflor_cancel_petal_drags"])
                Module["_ashflor_cancel_petal_drags"]();
        });
    });
    return 0;
}

void main_loop() {
    EM_ASM({
        function loop(time)
        {
            Module.ashflorDpr = Math.min(window.devicePixelRatio || 1, 2);
            const r = Module.ashflorDpr;
            const width = Math.max(1, Math.round(innerWidth * r));
            const height = Math.max(1, Math.round(innerHeight * r));
            if (Module.canvas.width !== width || Module.canvas.height !== height) {
                Module.canvas.width = width;
                Module.canvas.height = height;
            }
            const frameStart = Module['profileFrames'] ? performance.now() : 0;
            _loop(time, width, height);
            if (Module['profileFrames']) {
                const samples = Module['profileFrames'];
                samples.push(performance.now() - frameStart);
                if (samples.length === 300) {
                    const sorted = samples.slice().sort((a, b) => a - b);
                    const med = sorted[149].toFixed(1);
                    const p95 = sorted[284].toFixed(1);
                    const max = sorted[299].toFixed(1);
                    console.info('Ashflor frame work: median ' + med
                        + ' ms, P95 ' + p95 + ' ms, max ' + max
                        + ' ms (open with ?perf)');
                    samples.length = 0;
                }
            }
            if (Module['onFirstFrame']) {
                Module['onFirstFrame']();
                Module['onFirstFrame'] = null;
            }
            requestAnimationFrame(loop);
        };
        requestAnimationFrame(loop);
    });
}

int setup_canvas() {
    EM_ASM({
        Module.canvas = document.getElementById("canvas");
        Module.ashflorDpr = Math.min(window.devicePixelRatio || 1, 2);
        const r = Module.ashflorDpr;
        Module.canvas.width = Math.max(1, Math.round(innerWidth * r));
        Module.canvas.height = Math.max(1, Math.round(innerHeight * r));
        Module.canvas.oncontextmenu = function() { return false; };
        window.onbeforeunload = function(e) { return "Are you sure?"; };
        Module.ctxs = [];
        Module.availableCtxs = [];
        Module.TextDecoder = new TextDecoder('utf8');
    });
    return 0;
}

uint8_t check_mobile() {
    return EM_ASM_INT({
        return /iPhone|iPad|iPod|Android|BlackBerry/i.test(navigator.userAgent);
    });
}
