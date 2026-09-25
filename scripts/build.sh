#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cmake -S "$ROOT/analyzer" -B "$ROOT/build/analyzer" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build/analyzer" -j
ctest --test-dir "$ROOT/build/analyzer" --output-on-failure
if command -v mvn >/dev/null 2>&1; then
  (cd "$ROOT/sonar-plugin" && mvn clean package)
else
  echo "Maven not found; analyzer built/tested, Sonar plugin build skipped." >&2
fi
