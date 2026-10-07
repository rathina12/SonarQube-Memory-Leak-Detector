# Project Status and Release Gate (October 2026)

**Status: research prototype — not yet an industry-certified production analyzer.**

## Verified in GitHub Actions

- Default C++ analyzer Linux build, unit suite, and example syntax checks have passed on the audited PR branch.
- Windows and Maven/Sonar plugin checks are independent release gates; they must both pass before merging.
- A working SonarQube server end-to-end install, plugin-load and issue-import test is still required.

## Components to retain

- `analyzer/src`, `analyzer/include`: analysis engine, tokenization, reporting, config, CLI
- `analyzer/tests` and `examples`: executable regression coverage and reproducible fixtures
- `sonar-plugin`: report importer, Maven artifact, and importer tests
- `scripts`, `.github/workflows`: repeatable cross-platform builds
- `docs`: honest scope, limitation, installation, testing, and research methodology

## Not production implementations

- `analyzer/clang` is an **experimental event-hook skeleton**, not an AST/CFG-based detector. Merely compiling it cannot improve detection semantics; do not advertise it as a complete Clang frontend.
- `ML005` is listed but is **not currently emitted** by the default analyzer.
- `maxPathDepth` and `pathStatesMerged` do **not** represent a working bounded path exploration / real merge algorithm. Branches increment counters but do not fork executable memory states.
- `benchmark/results-template.csv` is a template, not measured performance evidence.
- `artifacts/sample-report.json` is an example output, not proof of current runtime behavior.

## Correctness gaps and release blockers

1. Analyzer executes branch bodies sequentially rather than exploring true/false paths, creating false leak reports and missed leaks.
2. Function ownership summaries are heuristics. Multiple returns, escaped pointers, allocation failure, pointer scopes/shadowing, and indirect calls are not fully modeled.
3. `realloc` behavior and C++ RAII/destructors require semantic handling.
4. File traversal errors, malformed/unreadable inputs, and hard failure paths need integration-level tests.
5. Validate SonarQube plugin compatibility on the exact supported server versions with imported finding locations.
6. Add labeled positive/negative benchmark fixtures and report precision, recall, F1, and runtime with reproducible baselines.

## Release policy

Do not call this project 100% accurate or production-ready based only on green builds. Require successful Linux + Windows + Maven CI, real SonarQube import smoke tests, parser/ownership regression testing, and benchmark evidence. Green CI proves only the covered cases.
