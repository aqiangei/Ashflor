#ifndef WASM_SERVER
#include <Server/Server.hh>

#include <Server/AccountStore.hh>
#include <Server/Client.hh>
#include <Server/LeaderboardStore.hh>
#include <Shared/Config.hh>

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <vector>

static us_listen_socket_t *g_listen_socket = nullptr;
static us_timer_t *g_delay_timer = nullptr;

uWS::App Server::server = uWS::App({
    .key_file_name = "misc/key.pem",
    .cert_file_name = "misc/cert.pem",
    .passphrase = "1234"
}).ws<Client>("/*", {
    /* Settings */
    .compression = uWS::DISABLED,
    .maxPayloadLength = 1024,
    // 120s: background mobile tabs may pause JS for a while; pings keep proxies happy
    .idleTimeout = 120,
    .maxBackpressure = 1024 * MAX_PACKET_LEN,
    .closeOnBackpressureLimit = false,
    .resetIdleTimeoutOnSend = true,
    .sendPingsAutomatically = true,
    /* Handlers */
    .upgrade = nullptr,
    .open = [](WebSocket *ws) {
        std::cout << "client connection\n";
        Client *client = ws->getUserData();
        client->ws = ws;
    },
    .message = [](WebSocket *ws, std::string_view message, uWS::OpCode opCode) {
        Client::on_message(ws, message, opCode);
    },
    .dropped = [](WebSocket *ws, std::string_view /*message*/, uWS::OpCode /*opCode*/) {
        std::cout << "dropped packet\n";
        Client *client = ws->getUserData();
        if (client == nullptr) {
            ws->end(1006, "Dropped Message");
            return;
        }
        client->disconnect();
        /* A message was dropped due to set maxBackpressure and closeOnBackpressureLimit limit */
    },
    .drain = [](WebSocket */*ws*/) {
        //assert(!1);
        /* Check ws->getBufferedAmount() here */
    },
    .close = [](WebSocket *ws, int code, std::string_view message) {
        Client::on_disconnect(ws, code, message);
    }
}).listen(SERVER_PORT, [](auto *listen_socket) {
    g_listen_socket = listen_socket;
    if (listen_socket) {
        std::cout << "Listening on port " << SERVER_PORT << std::endl;
    }
});

void Server::run() {
    struct sigaction sa;
    sa.sa_handler = [](int){ Server::is_draining = 1; };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGUSR2, &sa, nullptr);

    struct us_loop_t *loop = (struct us_loop_t *) uWS::Loop::get();
    g_delay_timer = us_create_timer(loop, 0, 0);

    us_timer_set(g_delay_timer, [](us_timer_t *x){ Server::tick(); }, 1, 1000 / TPS);
    Server::server.run();
}

void Server::stop() {
    if (Server::is_stopping) return;
    Server::is_stopping = true;
    std::cout << "stopping...\n";
    AccountStore::save();
    LeaderboardStore::flush();
    if (g_listen_socket != nullptr)
        us_listen_socket_close(0, g_listen_socket);
    if (g_delay_timer != nullptr)
        us_timer_close(g_delay_timer);
    // Kick remaining clients so uWS loop can exit
    if (Server::games != nullptr) {
        for (uint8_t i = 0; i < MapID::kNumMaps; ++i) {
            std::vector<Client *> snap(
                Server::games[i].clients.begin(),
                Server::games[i].clients.end());
            for (Client *c : snap) {
                if (c != nullptr)
                    c->disconnect(1001, "Shutting Down");
            }
        }
    }
}

void Client::send_packet(uint8_t const *packet, size_t size) {
    if (ws == nullptr) return;
    std::string_view message(reinterpret_cast<char const *>(packet), size);
    ws->send(message, uWS::OpCode::BINARY, 0);
}
#endif