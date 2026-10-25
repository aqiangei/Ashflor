#include <Client/Socket.hh>
#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>
#include <Client/Storage.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>

#include <cstring>
#include <iostream>

#include <emscripten.h>

uint8_t INCOMING_PACKET[64 * 1024] = {0};
uint8_t OUTGOING_PACKET[64 * 1024] = {0};
static uint8_t pending_guest_progress = 0;

extern "C" {
    void on_message(uint8_t type, uint32_t len, char *reason) {
        if (type == 0) {
            // WS open ≠ verified. Do NOT show wait/mini-game here — loading
            // keeps the original loader; wait screen only after disconnect once
            // the page has finished first paint (see index.html).
            std::printf("Connected\n");
            Game::disconnect_message.clear();
            Writer w(OUTGOING_PACKET);
            w.write<uint8_t>(Serverbound::kVerify);
            w.write<uint64_t>(VERSION_HASH);
            Game::reset();
            pending_guest_progress = 1;
            Game::socket.ready = 1; //force send
            Game::socket.send(w.packet, w.at - w.packet);
            Game::socket.ready = 0;
        } 
        else if (type == 2) {
            Game::on_game_screen = 0;
            Game::socket.ready = 0;
            pending_guest_progress = 0;
            std::printf("Disconnected [%d](%s)\n", len, reason);
            uint8_t intentional_leave =
                (len == 1000 && reason && std::strcmp(reason, "leave") == 0) ? 1 : 0;
            uint8_t kicked_elsewhere =
                (reason && std::strcmp(reason, "Logged in elsewhere") == 0) ? 1 : 0;
            if (kicked_elsewhere) {
                // Another login took this account — do not auto-resume and steal back.
                // Sticky notice survives Game::reset and shows in account_status
                // (same slot as wrong password).
                Game::session_token = 0;
                Game::account_logged_in = 0;
                Game::password.clear();
                Game::pending_auto_login = 0;
                Game::spawn_after_auth = 0;
                Game::pending_kick_notice = 1;
                Game::account_status = I18n::tr(I18n::kAuthAlreadyIn);
                Game::auth_panel = 1;
                Game::disconnect_message = Game::account_status;
                Storage::set();
            } else if (std::strlen(reason))
                Game::disconnect_message = I18n::format_disconnected(len, reason);
            else
                Game::disconnect_message = I18n::format_disconnected(len, "");
            if (!intentional_leave) {
                // 4003 = CloseReason::kOutdated — need hard refresh, not silent retry
                // 1006 = tab backgrounded / network drop
                // 1001 / 1012 = server going away / service restart (redeploy)
                // Kick: skip reconnect overlay — title account_status carries the tip.
                if (!kicked_elsewhere) {
                    char const *zh = (len == 4003)
                        ? "客户端版本过旧，正在自动刷新…"
                        : (len == 1006)
                            ? "已切到后台或网络中断，正在重连"
                            : (len == 1001 || len == 1012)
                                ? "服务器更新或停服，正在重连"
                                : "连接断开，正在重连";
                    EM_ASM({
                        if (Module.showReconnectOverlay)
                            Module.showReconnectOverlay(UTF8ToString($0));
                        if ($1 === 4003 && Module._ashflorSetReconnectDelay)
                            Module._ashflorSetReconnectDelay(8000);
                    }, zh, (int) len);
                }
                if (len == CloseReason::kOutdated)
                    DOM::update_client(1200);
            }
            free(reason);
        }
        else if (type == 1) {
            Game::socket.ready = 1;
            EM_ASM({
                if (Module.hideReconnectOverlay) Module.hideReconnectOverlay();
                if (Module._ashflorSetReconnectDelay)
                    Module._ashflorSetReconnectDelay(0);
            });
            // After verify: resume session → saved account login → guest save
            if (pending_guest_progress) {
                pending_guest_progress = 0;
                if (Game::pending_kick_notice) {
                    // Stay on title with the kick tip; do not auto-resume.
                    Game::send_guest_progress();
                    Game::account_status = I18n::tr(I18n::kAuthAlreadyIn);
                    Game::auth_panel = 1;
                } else if (Game::session_token != 0 && !Game::username.empty())
                    Game::account_resume();
                else if (!Game::username.empty() && !Game::password.empty()) {
                    Game::pending_auto_login = 1;
                    Game::account_status = I18n::use_chinese
                        ? "正在自动登录…" : "Signing in…";
                    Game::account_auth(0);
                } else
                    Game::send_guest_progress();
            }
            Game::on_message(INCOMING_PACKET, len);
        }
    }
}

Socket::Socket() {}

void Socket::connect(std::string const url) {
    std::cout << "Connecting to " << url << '\n';
    EM_ASM({
        let string = UTF8ToString($1);
        let hbOpcode = $2;
        let reconnectTimer = 0;
        let intentionalClose = false;
        let reconnectDelayOverride = 0; // ms; 0 = default
        let reconnectAttempt = 0;       // exponential backoff: 400ms → 15s cap
        let socketGen = 0; // ignore onclose from sockets we replaced

        function clearReconnectTimer() {
            if (reconnectTimer) {
                clearTimeout(reconnectTimer);
                reconnectTimer = 0;
            }
        }

        function sendHeartbeat() {
            try {
                if (!Module.socket || Module.socket.readyState !== 1) return;
                Module.socket.send(new Uint8Array([hbOpcode]));
            } catch (e) {}
        }

        function retireSocket(old) {
            if (!old) return;
            try {
                old.onopen = null;
                old.onclose = null;
                old.onerror = null;
                old.onmessage = null;
                if (old.readyState < 2) old.close();
            } catch (e) {}
        }

        function connect() {
            clearReconnectTimer();
            const gen = ++socketGen;
            retireSocket(Module.socket);
            let socket = Module.socket = new WebSocket(string);
            socket.binaryType = "arraybuffer";
            socket.onopen = function() {
                if (gen !== socketGen) return;
                _on_message(0, 0, 0);
            };
            socket.onclose = function(a) {
                // Replaced sockets often report 1006 — must not surface as a real drop
                if (gen !== socketGen) return;
                _on_message(2, a.code, stringToNewUTF8(a.reason || ""));
                if (intentionalClose) return;
                // Exponential backoff with jitter. A server that keeps rejecting us
                // (4003 outdated / 4001 too many connections / 1006 storms) must not
                // be hammered every 400ms: a stale tab used to reconnect ~2900
                // times a night and flood the access log.
                reconnectAttempt = Math.min(reconnectAttempt + 1, 8);
                let base = (typeof document !== "undefined" && document.visibilityState === "visible")
                    ? 400 : 1200;
                let delay = reconnectDelayOverride > 0
                    ? reconnectDelayOverride
                    : Math.min(base * Math.pow(2, reconnectAttempt - 1), 15000);
                delay = Math.round(delay * (0.85 + Math.random() * 0.3));
                clearReconnectTimer();
                reconnectTimer = setTimeout(connect, delay);
            };
            socket.onerror = function() {};
            socket.onmessage = function(event) {
                if (gen !== socketGen) return;
                HEAPU8.set(new Uint8Array(event.data), $0);
                _on_message(1, event.data.byteLength, 0);
            };
        }

        Module._ashflorSetReconnectDelay = function(ms) {
            reconnectDelayOverride = ms | 0;
            // Server accepted the session — next drop starts from the fast retry again
            if (reconnectDelayOverride === 0) reconnectAttempt = 0;
        };
        Module._ashflorReconnectNow = function() {
            intentionalClose = false;
            reconnectAttempt = 0;
            clearReconnectTimer();
            connect();
        };
        Module._ashflorMarkLeave = function() {
            intentionalClose = true;
            clearReconnectTimer();
        };

        // App heartbeat — setInterval still fires (throttled) when rAF is frozen
        if (!Module._ashflorHbStarted) {
            Module._ashflorHbStarted = 1;
            // Keepalive under background timer throttling (pairs with idleTimeout=300).
            setInterval(sendHeartbeat, 1500);
            document.addEventListener("visibilitychange", function() {
                if (document.visibilityState === "hidden") {
                    // Last chance keepalive before the browser freezes timers
                    sendHeartbeat();
                    return;
                }
                let s = Module.socket;
                if (!s || s.readyState === 2 || s.readyState === 3) {
                    Module._ashflorReconnectNow();
                } else if (s.readyState === 1) {
                    sendHeartbeat();
                }
            });
            // iOS pageshow on return — never interrupt CONNECTING (that caused fake 1006)
            window.addEventListener("pageshow", function() {
                let s = Module.socket;
                if (!s || s.readyState === 2 || s.readyState === 3)
                    Module._ashflorReconnectNow();
            });
        }

        setTimeout(connect, 400);
    }, INCOMING_PACKET, url.c_str(), (int)Serverbound::kHeartbeat);
}

void Socket::disconnect() {
    ready = 0;
    EM_ASM({
        try {
            if (Module._ashflorMarkLeave) Module._ashflorMarkLeave();
            if (Module.socket && Module.socket.readyState < 2)
                Module.socket.close(1000, "leave");
        } catch (e) {}
    });
}

void Socket::send(uint8_t *ptr, uint32_t len) {
    if (ready == 0) return; 
    // slice() copies — avoid HEAP overwrite racing an in-flight WS send
    EM_ASM({
        if (Module.socket?.readyState == 1) {
            Module.socket.send(HEAPU8.slice($0, $0+$1));
        }
    }, ptr, len);
}
