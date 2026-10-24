#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
compiler="${CXX:-clang++}"
out="${1:-/tmp/ashflor-feedback-tests}"
mkdir -p "$out"
"$compiler" -std=c++20 -O2 -DSERVERSIDE=1 -DASHFLOR_FEEDBACK_TEST=1 -ffunction-sections -fdata-sections -I. \
    tools/test_feedback_store.cc Server/FeedbackStore.cc Helpers/UTF8.cc \
    Shared/EntityDef.cc -Wl,--gc-sections -o "$out/feedback-store"
scratch="$(mktemp -d "$out/store.XXXXXX")"
cd "$scratch"
"$out/feedback-store"
