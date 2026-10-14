#!/usr/bin/env bash
# Zero-downtime ashflor deploy (WASM server + nginx version map).
# Requires: emscripten, cmake, make, node, nginx with map/location snippets installed.
#
# Env overrides:
#   WS_URL      default wss://ashflor.com/  (must match nginx location)
#   PULL=0      skip git pull
#   WASM_SERVER=1 (default)
#   DEBUG=0

set -eux

ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"

PULL=${PULL:-1}
if [[ "$PULL" -eq 1 ]]; then
    git pull --ff-only
    PULL=0 exec ./deploy.sh
fi

DEBUG=${DEBUG:-0}
WASM_SERVER=${WASM_SERVER:-1}
GENERAL_SPATIAL_HASH=${GENERAL_SPATIAL_HASH:-1}
USE_CODEPOINT_LEN=${USE_CODEPOINT_LEN:-1}
WS_URL=${WS_URL:-'wss://ashflor.com/'}
VERSION_HASH=$(date +%s)
SERVER_PORT=$(python3 -c 'import socket; s = socket.socket(); s.bind(("", 0)); print(s.getsockname()[1]); s.close()')
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
PID_FILE='Server/build/gardn-server.pid'
NGINX_DIR='Server/build/nginx'
NGINX_FILE="$NGINX_DIR/$VERSION_HASH.conf"

cmake -S Client -B Client/build \
    "-DDEBUG=$DEBUG" \
    "-DUSE_CODEPOINT_LEN=$USE_CODEPOINT_LEN" \
    "-DWS_URL=$WS_URL" \
    "-DVERSION_HASH=$VERSION_HASH"
cmake --build Client/build "-j$JOBS"

cmake -S Server -B Server/build \
    "-DDEBUG=$DEBUG" \
    "-DWASM_SERVER=$WASM_SERVER" \
    "-DGENERAL_SPATIAL_HASH=$GENERAL_SPATIAL_HASH" \
    "-DUSE_CODEPOINT_LEN=$USE_CODEPOINT_LEN" \
    "-DVERSION_HASH=$VERSION_HASH" \
    "-DSERVER_PORT=$SERVER_PORT"
cmake --build Server/build "-j$JOBS"

cd Server/build

# Serve client assets from the server build dir
cp -f ../../Client/public/* .
cp -f ../../Client/build/gardn-client.js ../../Client/build/gardn-client.wasm . 2>/dev/null || true
# Prefer compressed copies if present
cp -f ../../Client/build/gardn-client.js.br ../../Client/build/gardn-client.wasm.br . 2>/dev/null || true
cp -f ../../Client/build/gardn-client.js.gz ../../Client/build/gardn-client.wasm.gz . 2>/dev/null || true

# Stamp HTML so first visit after deploy does not use a stale localStorage hash alone
if [[ -f index.html ]]; then
    sed -i "s/var versionHash = '[^']*';/var versionHash = '$VERSION_HASH';/" index.html \
        || sed -i '' "s/var versionHash = '[^']*';/var versionHash = '$VERSION_HASH';/" index.html
fi

if [[ "$WASM_SERVER" -eq 1 ]]; then
    node gardn-server.js &
else
    ./gardn-server &
fi
NEW_PID=$!

sleep 3
kill -0 "$NEW_PID"

mkdir -p nginx
# nginx map: $arg_v → backend port (see deploy/nginx_map.conf)
echo "'$VERSION_HASH' $SERVER_PORT;" > "$NGINX_FILE"
echo "default $SERVER_PORT;" > nginx/default.conf
nginx -s reload

# Ask previous instance to drain (no new players; exit when empty)
if [[ -f "$PID_FILE" ]]; then
    OLD_PID=$(cat "$PID_FILE" || true)
    if [[ -n "${OLD_PID:-}" ]] && kill -0 "$OLD_PID" 2>/dev/null; then
        kill -s SIGUSR2 "$OLD_PID" || true
    fi
fi
echo "$NEW_PID" > "$PID_FILE"

# Cleanup nginx map when this process exits
(
    tail --pid "$NEW_PID" -f /dev/null
    rm -f "$NGINX_FILE"
    nginx -s reload || true
) &

echo "Deployed VERSION_HASH=$VERSION_HASH port=$SERVER_PORT pid=$NEW_PID"
