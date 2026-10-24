#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/Assets/QqLogo.hh>

#include <Helpers/UTF8.hh>
#include <Shared/Config.hh>

#include <cstdlib>

#include <emscripten.h>

namespace DOM {
    uint8_t text_enter_pressed = 0;
}

extern "C" void EMSCRIPTEN_KEEPALIVE ashflor_text_enter() {
    DOM::text_enter_pressed = 1;
}

void DOM::create_text_input(char const *name, uint32_t max_len, uint8_t password,
                            uint8_t dark) {
    EM_ASM(
    {
        const name = UTF8ToString($0);
        const elem = document.createElement('input');
        elem.id = name;
        elem.type = $2 ? "password" : "text";
        if ($3) elem.classList.add("chat-input");
        elem.style.position = "absolute";
        // Above #canvas (z-index:1) so typed text is visible and clickable
        elem.style.zIndex = "5";
        elem.style["font-family"] = "\"Noto Sans SC\", \"Microsoft YaHei\", \"Segoe UI\", sans-serif";
        elem.style["font-weight"] = "700";
        elem.style.display = 'none';
        elem.style["border-width"] = "0px";
        elem.style.background = "transparent";
        elem.style.border = "none";
        elem.style.outline = "none";
        elem.style.color = $3 ? "#f6f1ff" : "#111111";
        elem.style["caret-color"] = $3 ? "#d89aff" : "#111111";
        elem.style["-webkit-text-fill-color"] = $3 ? "#f6f1ff" : "#111111";
        elem.style["padding-left"] = "5px";
        elem.maxLength = ($1).toString();
        elem.setAttribute("spellcheck", "false");
        elem.setAttribute("autocomplete", $2 ? "current-password" : "off");
        elem.setAttribute("autocapitalize", "off");
        elem.setAttribute("autocorrect", "off");
        // "send" makes mobile soft-keyboards show a send/go action that fires Enter
        elem.setAttribute("enterkeyhint", $2 ? "done" : "send");
        // ≥16px prevents iOS page-zoom on focus (florr_clone mobile_text_input)
        elem.style["font-size"] = "16px";
        // Coalesce keydown + beforeinput + form submit into one flag per gesture
        let enterArmed = true;
        const notifyEnter = function () {
            if (!enterArmed) return;
            enterArmed = false;
            setTimeout(function () { enterArmed = true; }, 80);
            try {
                if (typeof Module._ashflor_text_enter === "function")
                    Module._ashflor_text_enter();
            } catch (err) {}
        };
        const isEnterKey = function (e) {
            if (e.isComposing || e.keyCode === 229) return false;
            return e.key === "Enter" || e.code === "Enter" || e.code === "NumpadEnter"
                || e.keyCode === 13;
        };
        // Soft-keyboard Send often never reaches window keydown; listen on the
        // overlay itself. Also catch beforeinput line-break + form submit —
        // common on iOS/Android when enterkeyhint=send.
        elem.addEventListener("keydown", function (e) {
            if (!isEnterKey(e)) return;
            e.preventDefault();
            notifyEnter();
        });
        elem.addEventListener("beforeinput", function (e) {
            if (e.inputType !== "insertLineBreak" && e.inputType !== "insertParagraph")
                return;
            e.preventDefault();
            notifyEnter();
        });
        if (!$2) {
            const form = document.createElement("form");
            form.setAttribute("action", "javascript:void(0)");
            form.style.display = "contents";
            form.addEventListener("submit", function (e) {
                e.preventDefault();
                notifyEnter();
            });
            form.appendChild(elem);
            document.body.appendChild(form);
        } else {
            document.body.appendChild(elem);
        }
    },
    name, max_len * 2, (int) password, (int) dark);
}

void DOM::set_input_password(char const *name, uint8_t password) {
    EM_ASM({
        const elem = document.getElementById(UTF8ToString($0));
        if (!elem) return;
        const start = elem.selectionStart;
        const end = elem.selectionEnd;
        elem.type = $1 ? "password" : "text";
        try { elem.setSelectionRange(start, end); } catch (e) {}
    }, name, (int) password);
}

void DOM::element_show(char const *name)
{
    EM_ASM(
    {
        const name = UTF8ToString($0);
        const elem = document.getElementById(name);
        elem.style.display = 'block';
    },
    name);
}

void DOM::element_hide(char const *name)
{
    EM_ASM(
    {
        const name = UTF8ToString($0);
        const elem = document.getElementById(name);
        elem.style.display = 'none';
    },
    name);
}

void DOM::update_pos_and_dimension(char const *name, float x, float y, float w, float h)
{
    EM_ASM(
        {
            const name = UTF8ToString($0);
            const elem = document.getElementById(name);
            const r = (typeof Module !== "undefined" && Module.ashflorDpr)
                ? Module.ashflorDpr
                : Math.min(window.devicePixelRatio || 1, 2);
            elem.style.left = ($1 - $3 / 2) / r + "px";
            elem.style.top = ($2 - $4 / 2) / r + "px";
            elem.style.width = ($3 / r - 10) + "px";
            elem.style.height = $4 / r + "px";
            // Keep ≥16px on mobile so iOS never zooms the page
            const fs = Math.max(16, $4 / r * 0.66);
            elem.style["font-size"] = fs + "px";
        },
        name, x, y, w, h);
}

std::string DOM::retrieve_text(char const *name, uint32_t max_length) {
    char *ptr = (char *) EM_ASM_PTR(
    {
        const name = UTF8ToString($0);
        const elem = document.getElementById(name);
        return stringToNewUTF8(elem.value);
    },
    name);
    std::string out{ptr};
    out = UTF8Parser::trunc_string(out, max_length);
    free(ptr);
    return out;
}

void DOM::update_text(char const *name, std::string const &contents, uint32_t max_length) {
    EM_ASM({
        const name = UTF8ToString($0);
        const elem = document.getElementById(name);
        elem.value = UTF8ToString($1).slice(0,$2);
    }, name, contents.c_str(), max_length);
}

void DOM::open_page(char const *url) {
    EM_ASM({
        try {
            window.open(UTF8ToString($0));
        } catch(e) {}
    }, url);
}

void DOM::open_feedback_board() {
    EM_ASM({
        const user = UTF8ToString($0);
        Module.feedbackSession = Object.create(null);
        Module.feedbackSession["kind"] = "ashflor-feedback-session";
        Module.feedbackSession["user"] = user;
        if (!Module.feedbackListener) {
            Module.feedbackListener = function(event) {
                if (event.origin !== location.origin || event.source !== Module.feedbackWindow
                    || !event.data) return;
                if (event.data["kind"] === "ashflor-feedback-ready")
                    event.source.postMessage(Module.feedbackSession, location.origin);
                else if (event.data["kind"] === "ashflor-feedback-post"
                    && typeof event.data["text"] === "string")
                    _ashflor_feedback_post(stringToNewUTF8(event.data["text"]));
                else if (event.data["kind"] === "ashflor-feedback-resolve"
                    && typeof event.data["id"] === "number")
                    _ashflor_feedback_resolve(
                        event.data["id"] >>> 0,
                        event.data["resolved"] ? 1 : 0);
            };
            window.addEventListener("message", Module.feedbackListener);
        }
        Module.feedbackWindow = window.open("/feedback.html", "ashflor-feedback");
        if (Module.feedbackWindow) {
            Module.feedbackWindow.postMessage(Module.feedbackSession, location.origin);
            Module.feedbackWindow.focus();
        }
    }, Game::account_logged_in ? Game::username.c_str() : "");
}

extern "C" EMSCRIPTEN_KEEPALIVE void ashflor_feedback_post(char *text) {
    Game::post_feedback(text ? text : "");
    std::free(text);
}

extern "C" EMSCRIPTEN_KEEPALIVE void ashflor_feedback_resolve(uint32_t id, uint8_t resolved) {
    Game::set_feedback_resolved(id, resolved);
}

void DOM::feedback_result(uint8_t result, uint32_t id) {
    EM_ASM({
        if (!Module.feedbackWindow || Module.feedbackWindow.closed) return;
        const reply = Object.create(null);
        reply["kind"] = "ashflor-feedback-result";
        reply["result"] = $0;
        reply["id"] = $1;
        Module.feedbackWindow.postMessage(reply, location.origin);
    }, result, id);
}

void DOM::feedback_resolve_result(uint8_t result, uint32_t id, uint8_t resolved) {
    EM_ASM({
        if (!Module.feedbackWindow || Module.feedbackWindow.closed) return;
        const reply = Object.create(null);
        reply["kind"] = "ashflor-feedback-resolved";
        reply["result"] = $0;
        reply["id"] = $1;
        reply["resolved"] = $2;
        Module.feedbackWindow.postMessage(reply, location.origin);
    }, result, id, resolved);
}

void DOM::open_qq_group() {
    EM_ASM({
        const old = document.getElementById("ashflor-qq-dialog");
        if (old) { old.focus(); return; }
        if (!document.getElementById("ashflor-qq-dialog-style")) {
            const style = document.createElement("style");
            style.id = "ashflor-qq-dialog-style";
            style.textContent =
                "#ashflor-qq-dialog{border:2px solid #d7bd79;border-radius:20px;padding:28px 28px 22px;"
                + "background:#fff9e9;color:#384638;font-family:system-ui,sans-serif;"
                + "width:min(340px,calc(100vw - 48px));max-width:calc(100vw - 48px);text-align:center;"
                + "box-shadow:0 18px 80px #0007}"
                + "#ashflor-qq-dialog::backdrop{background:rgba(20,28,22,.45)}"
                + "#ashflor-qq-dialog h2{margin:12px 0 8px;font-size:1.35rem}"
                + "#ashflor-qq-dialog p{margin:0 0 10px}"
                + "#ashflor-qq-dialog .qq-hint{font-size:14px;opacity:.85;line-height:1.4}"
                + "#ashflor-qq-dialog .qq-actions{display:flex;flex-direction:column;gap:10px;margin-top:18px}"
                + "#ashflor-qq-dialog .qq-row{display:flex;gap:10px}"
                + "#ashflor-qq-dialog .qq-btn{"
                + "display:inline-flex;align-items:center;justify-content:center;box-sizing:border-box;"
                + "width:100%;min-height:44px;padding:11px 16px;border-radius:12px;font-size:15px;"
                + "font-weight:600;cursor:pointer;text-decoration:none;border:2px solid transparent;"
                + "-webkit-tap-highlight-color:transparent;user-select:none}"
                + "#ashflor-qq-dialog .qq-btn-primary{background:#48a575;color:#fff;border-color:#3a8a60}"
                + "#ashflor-qq-dialog .qq-btn-primary:hover{background:#3f9668}"
                + "#ashflor-qq-dialog .qq-btn-secondary{background:#f3ead3;color:#384638;border-color:#d7bd79}"
                + "#ashflor-qq-dialog .qq-btn-secondary:hover{background:#ebe0c4}"
                + "#ashflor-qq-dialog [data-status]{min-height:20px;margin:10px 0 0;font-size:14px}";
            document.head.appendChild(style);
        }
        const dialog = document.createElement("dialog");
        dialog.id = "ashflor-qq-dialog";
        dialog.innerHTML =
            '<img alt="QQ" width="52" height="52">'
            + '<h2>Ashflor 交流群</h2>'
            + '<p>QQ群：<strong>227990150</strong></p>'
            + '<p class="qq-hint">打不开 QQ 时，可以复制群号搜索加入。</p>'
            + '<div class="qq-actions">'
            +   '<a class="qq-btn qq-btn-primary" href="https://qm.qq.com/q/E0OG9CLIg" target="_blank" rel="noopener noreferrer">加入 QQ 群</a>'
            +   '<div class="qq-row">'
            +     '<button type="button" class="qq-btn qq-btn-secondary" data-copy>复制群号</button>'
            +     '<button type="button" class="qq-btn qq-btn-secondary" data-close>关闭</button>'
            +   '</div>'
            + '</div>'
            + '<p data-status role="status"></p>';
        dialog.querySelector("img").src = UTF8ToString($0);
        dialog.querySelector("[data-copy]").onclick = async function() {
            const status = dialog.querySelector("[data-status]");
            try { await navigator.clipboard.writeText("227990150"); status.textContent = "群号已复制"; }
            catch (e) { status.textContent = "请手动复制群号：227990150"; }
        };
        dialog.querySelector("[data-close]").onclick = function() { dialog.close(); };
        // Backdrop / outside the panel → dismiss (native <dialog> click target is the dialog itself).
        dialog.addEventListener("click", function(event) {
            if (event.target === dialog) dialog.close();
        });
        dialog.onclose = function() { dialog.remove(); };
        for (const name of new Array("keydown", "keyup", "mousedown", "mouseup", "touchstart", "touchend", "touchmove"))
            dialog.addEventListener(name, function(event) { event.stopPropagation(); });
        document.body.appendChild(dialog);
        dialog.showModal();
    }, QQ_LOGO_DATA);
}

void DOM::focus_element(char const *name) {
    EM_ASM({
        const elem = document.getElementById(UTF8ToString($0));
        if (!elem) return;
        // Synchronous focus from tap/click opens the mobile soft keyboard
        elem.focus({ preventScroll: true });
        try { elem.setSelectionRange(elem.value.length, elem.value.length); } catch (e) {}
    }, name);
}

void DOM::blur_active() {
    EM_ASM({
        if (document.activeElement && document.activeElement.blur)
            document.activeElement.blur();
    });
}

void DOM::cache_client() {
    EM_ASM({
        try {
            window.localStorage["version_hash"] = $0.toString();
        } catch (e) {}
        window.onerror = window.onunhandledrejection = null;
    }, (double) VERSION_HASH);
}

void DOM::update_client(float delay_ms) {
    EM_ASM({
        try {
            delete window.localStorage["version_hash"];
        } catch (e) {}
        window.onbeforeunload = null;
        setTimeout(function () {
            window.location.reload();
        }, $0);
    }, delay_ms);
}

void DOM::toggle_fullscreen() {
    EM_ASM({
        try {
            if (!document.fullscreenElement && !document.webkitFullscreenElement) {
                var el = document.documentElement;
                if (el.requestFullscreen) el.requestFullscreen();
                else if (el.webkitRequestFullscreen) el.webkitRequestFullscreen();
            } else if (document.exitFullscreen) {
                document.exitFullscreen();
            } else if (document.webkitExitFullscreen) {
                document.webkitExitFullscreen();
            }
        } catch (e) {}
    });
}
