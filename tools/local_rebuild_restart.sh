#!/usr/bin/env bash
set -euo pipefail
source /root/emsdk/emsdk_env.sh >/dev/null
cd /mnt/d/ashflor/ashflor

fuser -k 9001/tcp 2>/dev/null || true
pkill -f 'node ./gardn-server.js' 2>/dev/null || true
sleep 1

emmake cmake --build Client/build -j"$(nproc)"
emmake cmake --build Server/build -j"$(nproc)"
cp -f Client/build/gardn-client.js Client/build/gardn-client.wasm Client/public/
node Scripts/compress_client.js Client/public
cp -f Client/public/gardn-client.js Client/public/gardn-client.wasm \
  Client/public/gardn-client.js.br Client/public/gardn-client.js.gz \
  Client/public/gardn-client.wasm.br Client/public/gardn-client.wasm.gz \
  Server/build/
echo BUILD_OK
