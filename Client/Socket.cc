#include <Client/Socket.hh>
#include <Client/DOM.hh>
#include <Client/Game.hh>
#include <Client/I18n.hh>

#include <Shared/Binary.hh>
#include <Shared/Config.hh>

#include <cstring>
#include <iostream>

#include <emscripten.h>

uint8_t INCOMING_PACKET[64 * 1024] = {0};
uint8_t OUTGOING_PACKET[1 * 1024] = {0};
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
            if (std::strlen(reason))
                Game::disconnect_message = I18n::format_disconnected(len, reason);
            else
                Game::disconnect_message = I18n::format_disconnected(len, "");
            if (!intentional_leave) {
                // 4003 = CloseReason::kOutdated — need hard refresh, not silent retry
                // 1006 = tab backgrounded / network drop
                // 1001 / 1012 = server going away / service restart (redeploy)
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
                    // Slow auto-retry on outdated so tip stays readable before reload
                    if ($1 === 4003 && Module._ashflorSetReconnectDelay)
                        Module._ashflorSetReconnectDelay(8000);
                }, zh, (int) len);
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
            // After verify: resume browser session, else restore guest save
            if (pending_guest_progress) {
                pending_guest_progress = 0;
                if (Game::session_token != 0 && !Game::username.empty())
                    Game::account_resume();
                else
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
                let delay = reconnectDelayOverride > 0
                    ? reconnectDelayOverride
                    : ((typeof document !== "undefined" && document.visibilityState === "visible")
                        ? 400 : 1200);
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
        };
        Module._ashflorReconnectNow = function() {
            intentionalClose = false;
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
            setInterval(sendHeartbeat, 4000);
            document.addEventListener("visibilitychange", function() {
                if (document.visibilityState !== "visible") return;
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
