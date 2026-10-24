#!/usr/bin/env bash
set -euo pipefail
source /root/emsdk/emsdk_env.sh >/dev/null
cd /mnt/d/ashflor/ashflor/Server/build
# only kill listeners on 9001, not by process name (avoids racing ourselves)
if command -v fuser >/dev/null; then
  fuser -k 9001/tcp 2>/dev/null || true
  sleep 1
fi
exec node ./gardn-server.js
