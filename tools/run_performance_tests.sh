#!/usr/bin/env bash
# Run native regressions with the production floating-point flags.
# From WSL: CXX=/root/emsdk/upstream/bin/clang++ bash tools/run_performance_tests.sh
set -euo pipefail
cd "$(dirname "$0")/.."
compiler="${CXX:-clang++}"
out="${1:-/tmp/ashflor-perf-tests}"
mkdir -p "$out"
opts=(-std=c++20 -O3 -ffast-math -Wno-narrowing -Wno-reorder-init-list -I.
      -ffunction-sections -fdata-sections -Wl,--gc-sections)
shared=(Shared/MapGen.cc Shared/StaticData.cc Helpers/Math.cc)
"$compiler" "${opts[@]}" tools/perf_terrain.cc "${shared[@]}" -o "$out/terrain" 2> "$out/terrain-build.log"
"$out/terrain" | tee "$out/terrain.log"
"$compiler" "${opts[@]}" -DSERVERSIDE=1 tools/perf_spawn.cc "${shared[@]}" \
    Shared/Simulation.cc Shared/Entity.cc Shared/EntityDef.cc Shared/Arena.cc \
    Shared/Binary.cc Helpers/Vector.cc Helpers/UTF8.cc Server/SpatialHashCanonical.cc \
    -o "$out/spawn" 2> "$out/spawn-build.log"
"$out/spawn" | tee "$out/spawn.log"
"$compiler" "${opts[@]}" -DCLIENTSIDE=1 tools/perf_particles.cc \
    Helpers/Math.cc Helpers/Vector.cc Shared/EntityDef.cc \
    -o "$out/particles" 2> "$out/particles-build.log"
"$out/particles" | tee "$out/particles.log"
"$compiler" "${opts[@]}" -DSERVERSIDE=1 -DWASM_SERVER=1 tools/perf_collision.cc \
    Shared/Entity.cc Shared/EntityDef.cc Shared/Simulation.cc Shared/Arena.cc \
    Shared/Recipe.cc Shared/StaticData.cc Server/SpatialHashCanonical.cc Server/PetalTracker.cc \
    Helpers/Vector.cc Helpers/Math.cc -o "$out/collision" 2> "$out/collision-build.log"
"$out/collision" | tee "$out/collision.log"
"$compiler" "${opts[@]}" -DSERVERSIDE=1 tools/perf_network_view.cc \
    Shared/Binary.cc Shared/EntityDef.cc Helpers/Math.cc \
    -o "$out/network-view" 2> "$out/network-view-build.log"
"$out/network-view" | tee "$out/network-view.log"
