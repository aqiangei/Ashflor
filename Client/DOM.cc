#include <Client/DOM.hh>

#include <Helpers/UTF8.hh>
#include <Shared/Config.hh>

#include <emscripten.h>

void DOM::create_text_input(char const *name, uint32_t max_len, uint8_t password) {
    EM_ASM(
    {
        const name = UTF8ToString($0);
        const elem = document.createElement('input');
        elem.id = name;
        elem.type = $2 ? "password" : "text";
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
        elem.style.color = "#111111";
        elem.style["caret-color"] = "#111111";
        elem.style["-webkit-text-fill-color"] = "#111111";
        elem.style["padding-left"] = "5px";
        elem.maxLength = ($1).toString();
        elem.setAttribute("spellcheck", "false");
        elem.setAttribute("autocomplete", $2 ? "current-password" : "off");
        elem.setAttribute("autocapitalize", "off");
        elem.setAttribute("autocorrect", "off");
        elem.setAttribute("enterkeyhint", $2 ? "done" : "go");
        // ≥16px prevents iOS page-zoom on focus (florr_clone mobile_text_input)
        elem.style["font-size"] = "16px";
        document.body.appendChild(elem);
    },
    name, max_len * 2, (int) password);
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