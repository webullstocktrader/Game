#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
g++ -std=c++17 -Wall -Wextra -Werror -O0 -g \
	-I"$ROOT/Source/ValleyGod/Sim" \
	"$ROOT/Tests/test_valley_sim.cpp" \
	"$ROOT/Source/ValleyGod/Sim/ValleySim.cpp" \
	"$ROOT/Source/ValleyGod/Sim/ValleyPalette.cpp" \
	"$ROOT/Source/ValleyGod/Sim/ValleyLookPaths.cpp" \
	-o "$ROOT/Tests/test_valley_sim"
"$ROOT/Tests/test_valley_sim"
