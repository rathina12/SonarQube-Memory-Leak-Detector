#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT/scripts/build.sh"
find "$ROOT/examples" -name '*.c' -print0 | while IFS= read -r -d '' f; do clang -std=c17 -Wall -Wextra -Werror -fsyntax-only "$f"; done
find "$ROOT/examples" -name '*.cpp' -print0 | while IFS= read -r -d '' f; do clang++ -std=c++17 -Wall -Wextra -Werror -fsyntax-only "$f"; done
"$ROOT/build/analyzer/mlpca-analyzer" "$ROOT/examples" --output "$ROOT/build/mlpca-report.json"
python3 -m json.tool "$ROOT/build/mlpca-report.json" >/dev/null
