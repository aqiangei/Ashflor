#include <Client/Socket.hh>
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
            std::printf("Connected\n");
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
            if (std::strlen(reason))
                Game::disconnect_message = I18n::format_disconnected(len, reason);
            else
                Game::disconnect_message = I18n::format_disconnected(len, "");
            free(reason);
        }
        else if (type == 1) {
            Game::socket.ready = 1;
            // Send guest save after verify succeeded (first server packet)
            if (pending_guest_progress) {
                pending_guest_progress = 0;
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
        function connect() {
            let socket = Module.socket = new WebSocket(string);
            socket.binaryType = "arraybuffer";
            socket.onopen = function() {
                _on_message(0, 0, 0);
            };
            socket.onclose = function(a) {
                _on_message(2, a.code, stringToNewUTF8(a.reason));
                setTimeout(connect, 1000);
            };
            socket.onmessage = function(event) {
                HEAPU8.set(new Uint8Array(event.data), $0);
                _on_message(1, event.data.byteLength, 0);
            };
        }
        setTimeout(connect, 1000);
    }, INCOMING_PACKET, url.c_str());
}

void Socket::disconnect() {
    ready = 0;
    EM_ASM({
        try {
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