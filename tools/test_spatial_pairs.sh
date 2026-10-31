#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
compiler="${CXX:-clang++}"
out="${1:-/tmp/ashflor-spatial-tests}"
mkdir -p "$out"
"$compiler" -std=c++20 -O3 -ffast-math -DSERVERSIDE=1 \
    -Wno-narrowing -Wno-reorder-init-list -I. \
    -ffunction-sections -fdata-sections -Wl,--gc-sections \
    tools/perf_spatial_pairs.cc Shared/Entity.cc Shared/EntityDef.cc \
    Shared/Simulation.cc Shared/Arena.cc Shared/StaticData.cc \
    Helpers/Vector.cc Helpers/Math.cc Server/SpatialHashCanonical.cc \
    -o "$out/spatial-pairs" 2> "$out/build.log"
"$out/spatial-pairs"
