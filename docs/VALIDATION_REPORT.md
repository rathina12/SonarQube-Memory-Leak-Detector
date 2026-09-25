# Validation Report — v1.0.0

Validation environment: Linux, GCC 14.2, Clang 17, CMake 3.31, OpenJDK 21.

## Completed in this build

- C++17 analyzer Release build: **PASS**
- Analyzer regression tests: **14/14 PASS**
- AddressSanitizer + UndefinedBehaviorSanitizer regression run: **PASS**
- C fixtures compiled with Clang C17, `-Wall -Wextra -Werror`: **PASS**
- C++ fixtures compiled with Clang C++17, `-Wall -Wextra -Werror`: **PASS**
- Repository fixture scan: **10 files scanned, 0 failed**
- Expected defective-fixture findings: **7**
- JSON report parse/contract sanity: **PASS**
- Custom allocator/deallocator configuration: **PASS**
- Ownership-sink configuration: **PASS**
- `--fail-on critical` exit behavior: **PASS**
- Deterministic repeated report generation: **PASS**
- Java plugin production source syntax/stub compilation: **PASS**

## Fixture findings observed

- `simple_leak` -> ML001
- `overwrite` -> ML002
- `branch_leak` -> ML004
- `double_free` -> ML006
- `mismatch` -> ML007
- `realloc_misuse` -> ML008
- `use_after_free` -> ML009

The `safe`, `alias_safe`, and `interprocedural` fixtures produce no finding in the validated run.

## Environment-dependent validation still required after GitHub push

The current execution environment has no Maven installation and cannot resolve Maven Central, so the real SonarQube plugin JAR dependency build could not be executed here. The source was checked against the current Sonar external-issue API shape and syntax-compiled with API-compatible stubs. `.github/workflows/ci.yml` performs the real `mvn -B clean verify` build on GitHub Actions. A release should be tagged only after that CI job passes.

The optional `MLPCA_WITH_CLANG=ON` target requires LLVM/Clang development headers/libraries, which are not installed in this validation environment. The default analyzer is fully built/tested; the Clang adapter remains an optional semantic-integration layer and should receive a dedicated LLVM-enabled CI job before making claims based on Clang AST/CFG mode.
