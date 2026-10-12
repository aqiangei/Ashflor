#ifdef WASM_SERVER
#include <Server/Client.hh>
#include <Server/Server.hh>

#include <Shared/Config.hh>

#include <iostream>
#include <string>
#include <unordered_map>

#include <emscripten.h>

std::unordered_map<int, WebSocket *> WS_MAP;

size_t const MAX_BUFFER_LEN = 1024;
static uint8_t INCOMING_BUFFER[MAX_BUFFER_LEN] = {0};

extern "C" {
    void on_connect(int ws_id) {
        std::printf("client connect: [%d]\n", ws_id);
        WebSocket *ws = new WebSocket(ws_id);
        WS_MAP.insert({ws_id, ws});
    }

    void on_disconnect(int ws_id, int reason) {
        auto iter = WS_MAP.find(ws_id);
        if (iter == WS_MAP.end()) {
            std::printf("unknown ws disconnect: [%d]", ws_id);
            return;
        }
        std::printf("client disconnect: [%d]\n", ws_id);
        Client::on_disconnect(iter->second, reason, {});
        delete iter->second;
        WS_MAP.erase(ws_id);
    }

    void tick() {
        Server::tick();
    }

    void on_message(int ws_id, uint32_t len) {
        auto iter = WS_MAP.find(ws_id);
        //WebSocket *ws = WS_MAP[ws_id];
        if (iter == WS_MAP.end()) return;
        std::string_view message(reinterpret_cast<char const *>(INCOMING_BUFFER), len);
        Client::on_message(iter->second, message, 0);
    }
}

WebSocketServer::WebSocketServer() {
    EM_ASM({
        const WSS = require("ws");
        const http = require("http");
        const fs = require("fs");
        const server = http.createServer(function(req, res) {
            let encodeType = "text/html; charset=utf-8";
            let file = "index.html";
            // Strip ?query so cache-busting URLs still resolve
            const path = (req.url || "/").split("?")[0];
            switch (path) {
                case "/":
                    break;
                case "/gardn-client.js":
                    encodeType = "application/javascript; charset=utf-8";
                    file = "gardn-client.js";
                    break;
                case "/gardn-client.wasm":
                    encodeType = "application/wasm";
                    file = "gardn-client.wasm";
                    break;
                case "/sw.js":
                    encodeType = "application/javascript; charset=utf-8";
                    file = "sw.js";
                    break;
                default:
                    file = "";
                    break;
            }
            if (file && fs.existsSync(file)) {
                // HTML/SW: revalidate. JS/WASM: long-cache (URLs already bust with ?v=)
                const cacheCtl = (file === "index.html" || file === "sw.js")
                    ? "no-cache"
                    : "public, max-age=31536000, immutable";
                const headers = {
                    "Content-Type": encodeType,
                    "Cache-Control": cacheCtl
                };
                let servedFile = file;
                if (file === "gardn-client.js" || file === "gardn-client.wasm") {
                    const accepted = req.headers["accept-encoding"] || "";
                    const accepts = encoding => accepted.split(",").some(part => {
                        const fields = part.trim().split(";");
                        return fields[0] === encoding
                            && !fields.slice(1).some(option => Number(option.trim().split("=")[1]) === 0);
                    });
                    headers["Vary"] = "Accept-Encoding";
                    if (accepts("br") && fs.existsSync(file + ".br")) {
                        servedFile = file + ".br";
                        headers["Content-Encoding"] = "br";
                    } else if (accepts("gzip") && fs.existsSync(file + ".gz")) {
                        servedFile = file + ".gz";
                        headers["Content-Encoding"] = "gzip";
                    }
                }
                if (file === "sw.js")
                    headers["Service-Worker-Allowed"] = "/";
                fs.readFile(servedFile, function(err, body) {
                    if (err) {
                        res.writeHead(500, {"Content-Type": "text/plain; charset=utf-8"});
                        res.end("Read failed");
                        return;
                    }
                    headers["Content-Length"] = body.length;
                    res.writeHead(200, headers);
                    res.end(body);
                });
                return;
            }
            res.writeHead(404, {"Content-Type": "text/plain; charset=utf-8"});
            res.end("Not Found");
        });

        server.listen($0, function() {
            console.log("Server running at http://localhost:"+$0);
        });
        
        const wss = new WSS.Server({ "server": server });
        Module.ws_connections = {};
        let curr_id = 0;
        wss.on("connection", function(ws, req) {
            const ws_id = curr_id;
            Module.ws_connections[ws_id] = ws;
            _on_connect(ws_id);
            curr_id = (curr_id + 1) | 0;
            ws.on("message", function(message) {
                let data = new Uint8Array(message);
                const len = data.length > $2 ? $2 : data.length;
                data = data.subarray(0, len);
                HEAPU8.set(data, $1);
                _on_message(ws_id, len);
            });
            ws.on("close", function(reason) {
                _on_disconnect(ws_id, reason);
                delete Module.ws_connections[ws_id];
            });
        })
    }, SERVER_PORT, INCOMING_BUFFER, MAX_BUFFER_LEN);
}

void Server::run() {
    EM_ASM({
        setInterval(_tick, $0);
    }, 1000 / TPS);
}

void Client::send_packet(uint8_t const *packet, size_t size) {
    if (ws == nullptr) return;
    ws->send(packet, size);
}

WebSocket::WebSocket(int id) : ws_id(id) {
    client.ws = this;
}

void WebSocket::send(uint8_t const *packet, size_t size) {
    EM_ASM({
        if (!Module.ws_connections || !Module.ws_connections[$0]) return;
        const ws = Module.ws_connections[$0];
        ws.send(HEAPU8.subarray($1,$1+$2));
    }, ws_id, packet, size);
}

void WebSocket::end(int code, std::string const &message) {
    EM_ASM({
        if (!Module.ws_connections || !Module.ws_connections[$0]) return;
        const ws = Module.ws_connections[$0];
        ws.close($1, UTF8ToString($2));
    }, ws_id, code, message.c_str());
}

Client *WebSocket::getUserData() {
    return &client;
}

WebSocketServer Server::server;
#endif
