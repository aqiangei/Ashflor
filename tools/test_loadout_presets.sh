#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
compiler="${CXX:-clang++}"
out="${1:-/tmp/ashflor-loadout-tests}"
mkdir -p "$out"
"$compiler" -std=c++20 -O2 -Wno-narrowing -Wno-reorder-init-list -I. \
    -ffunction-sections -fdata-sections tools/test_loadout_presets.cc \
    Shared/StaticData.cc Shared/Recipe.cc Helpers/Math.cc -Wl,--gc-sections \
    -o "$out/loadout-presets" 2> "$out/build.log"
"$out/loadout-presets"
