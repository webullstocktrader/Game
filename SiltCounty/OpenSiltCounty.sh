#!/usr/bin/env bash
# Quotes matter when the project path contains spaces.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
PROJ="${ROOT}/SiltCounty.uproject"

if [[ $# -ge 1 ]]; then
  exec "$1" "$PROJ"
fi

echo "Open this project in Unreal Editor 5.4 or newer:"
echo "  \"${PROJ}\""
echo
echo "Example:"
echo "  \"/path/to/UnrealEditor\" \"${PROJ}\""
