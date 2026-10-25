#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
compiler="${CXX:-clang++}"
if ! command -v "$compiler" >/dev/null && [[ -x /root/emsdk/upstream/bin/clang++ ]]; then
    compiler=/root/emsdk/upstream/bin/clang++
fi
out="${1:-/tmp/ashflor-petal-mechanics}"
mkdir -p "$out"
"$compiler" -std=c++20 -O1 -DSERVERSIDE=1 -DWASM_SERVER=1 \
    -Wno-narrowing -Wno-reorder-init-list -ffunction-sections -fdata-sections -I. \
    tools/test_petal_mechanics.cc Server/EntityFunctions/Death.cc \
    Server/EntityFunctions/Damage.cc Server/EntityFunctions/Detection.cc \
    Server/EntityFunctions/Misc.cc Server/Process/Collision.cc Server/Process/Flower.cc \
    Server/Process/Petal.cc Server/Process/Health.cc \
    Server/Process/Curse.cc \
    Server/TrainingDummy.cc Server/Process/Motion.cc \
    Server/Spawn.cc Server/PetalTracker.cc Server/TeamManager.cc \
    Server/SpatialHashCanonical.cc Shared/Entity.cc Shared/EntityDef.cc \
    Shared/Simulation.cc Shared/Arena.cc Shared/Binary.cc Shared/Recipe.cc \
    Shared/StaticData.cc Shared/Map.cc Shared/MapGen.cc Shared/Craft.cc \
    Helpers/Vector.cc Helpers/Math.cc Helpers/UTF8.cc -Wl,--gc-sections \
    -o "$out/petal-mechanics" 2> "$out/build.log"
"$out/petal-mechanics"
