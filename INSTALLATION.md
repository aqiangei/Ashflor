
# Requirements
Please download the latest version of [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) and [CMake](https://cmake.org/download/). You will also need the latest version of gcc/g++ (>=c++20).

# Installation

## Native server (more performant):
```
> git clone --recurse-submodules https://github.com/trigonal-bacon/gardn.git
```
You will need to compile uWebSockets first. For in-depth complation options please visit the [uWebSockets installation page](https://github.com/uNetworking/uWebSockets/tree/master).
```
> cd gardn/Server/uWebSockets
> make
```
Then,
```
> cd gardn/Server
> mkdir build
> cd build
> cmake ..
> make
> ./gardn-server
```

## WebAssembly Server (doesn't require uWebSockets, but requires [Node.js](https://nodejs.org/en/download))
```
> git clone https://github.com/trigonal-bacon/gardn.git
> cd gardn/Server
> mkdir build
> cd build
> cmake .. -DWASM_SERVER=1
> make
> npm install ws fs http
> node ./gardn-server.js
```

## Client:
```
cd gardn/Client
mkdir build
cd build
cmake ..
make
```
Build with `DEBUG=0` for deployment. When Node.js is installed, the release build also creates `.br` and `.gz` copies of the JS and WASM files. Move all six files into `Client/public` (or `Server/build` for the WASM server), together with `index.html` and `sw.js`. If compression files are missing, run `node Scripts/compress_client.js Client/public` from the repository root after copying the JS and WASM files.

## Production deploy (recommended)

`./deploy.sh` builds client + WASM server with a timestamp `VERSION_HASH`, starts on a free port, updates nginx map files under `Server/build/nginx/`, and sends `SIGUSR2` to the previous process so it drains and exits when empty.

1. Install nginx snippets from `deploy/nginx_map.conf` (http block) and `deploy/nginx_location.conf` (server block); fix the `include` path.
2. Set `WS_URL` if your public URL is not `wss://ashflor.com/` (example: `WS_URL='wss://example.com/' ./deploy.sh`).
3. Defaults: `USE_CODEPOINT_LEN=1`, `GENERAL_SPATIAL_HASH=1` (Chinese string length + large-entity collision).

Client `index.html` uses `versionHash` + `localStorage.version_hash` for cache-busting; outdated clients auto-reload. Manual `cacheV` edits are no longer required when using `deploy.sh`.

The WASM server serves Brotli or gzip copies when supported by the browser. With nginx or another static host, configure it to serve precompressed `.br`/`.gz` files with the original JS/WASM MIME type and `Content-Encoding`, and keep `index.html` and `sw.js` revalidated.

For a quick browser measurement, append `?perf=1` to the game URL and inspect the console. It reports time to the first rendered frame and median/P95 main-thread frame work every 300 frames. Compare first visit and repeat visit in the same browser, then compare a crowded battle with a quiet scene.

The server is served by default at ``localhost:9001``. You may change the port by modifying ``Shared/Config.cc``

# Hosting 
The client may be hosted with any http server (eg. ``nginx``, ``http-server``). The wasm server automatically hosts content at ``localhost:9001`` as well.

If hosting somewhere other than ``localhost``, use the  ``WS_URL`` constant in ``Shared/Config.cc`` to specify a websocket url.

# Compilation Flags

``DEBUG`` | ``Server & Client`` | ``Default: 0`` : compiles with assertions and failsafes. <br>
``WASM_SERVER`` | ``Server only`` | ``Default : 0`` : compiles to WASM/JS instead of a native binary. <br>
``TDM`` | ``Server only`` | ``Default: 0`` : enables TDM instead of FFA.<br>
``GENERAL_SPATIAL_HASH`` | ``Server only`` | ``Default: 1`` : uses the canonical hash grid implementation instead of a uniform grid; enable this to support large entities. <br>
``USE_CODEPOINT_LEN`` | ``Server & Client`` | ``Default: 1`` : uses the number of codepoints (characters) instead of byte length for string validation and truncation - useful for non-english characters. Should be the same on both server and client. <br>
``VERSION_HASH`` / ``SERVER_PORT`` / ``WS_URL`` | cmake defines used by ``deploy.sh`` for zero-downtime rollouts.

# License
[LICENSE](./LICENSE)
