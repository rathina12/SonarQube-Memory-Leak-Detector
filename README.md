# MLPCA SonarQube Plugin

**Memory Leak Detection using Memory-Lifecycle Partial Call-Path Analysis (MLPCA)** for C/C++ projects with SonarQube integration.

## What this repository contains

- `analyzer/` — C++17 static analyzer core.
- `analyzer/clang/` — optional Clang LibTooling integration layer.
- `sonar-plugin/` — Java SonarQube sensor that imports analyzer findings as external issues.
- `examples/` — deterministic C/C++ leak and safe-code fixtures.
- `benchmark/` — benchmark runner and result schema.
- `docs/` — architecture, threat model, limitations, testing and deployment notes.
- `config/` — custom allocator/deallocator configuration example.

## Quick start

### 1. Build analyzer

```bash
cmake -S analyzer -B build/analyzer -DCMAKE_BUILD_TYPE=Release
cmake --build build/analyzer -j
ctest --test-dir build/analyzer --output-on-failure
```

### 2. Analyze a project

```bash
./build/analyzer/mlpca-analyzer examples --output mlpca-report.json
```

### 3. Build SonarQube plugin

Requires Maven and JDK 17+.

```bash
cd sonar-plugin
mvn clean package
```

Copy the resulting JAR from `target/` into the SonarQube `extensions/plugins/` directory and restart SonarQube.

Set:

```properties
sonar.mlpca.reportPaths=mlpca-report.json
```

Then run the normal Sonar scanner.

## Analyzer rules

| Rule | Description |
|---|---|
| ML001 | Allocated memory reaches function/project exit without release |
| ML002 | Pointer overwritten while owning unreleased allocation |
| ML003 | Early-return leak |
| ML004 | Conditional-path leak |
| ML005 | Ownership escapes or crosses a function boundary unsafely |
| ML006 | Double free/delete |
| ML007 | Mismatched allocation/deallocation family |
| ML008 | Unsafe `realloc` assignment that can lose the original allocation |
| ML009 | Use after free |
| ML010 | Potential repeated allocation in a loop without release |

## Design goals

1. Keep the analyzer core independent from SonarQube.
2. Track allocation objects rather than only variable names, so aliases can be handled.
3. Merge equivalent memory states to reduce redundant path exploration.
4. Produce deterministic JSON that can be consumed by SonarQube or other tools.
5. Fail soft on unsupported files: one parse failure must not abort a repository scan.

See `docs/ARCHITECTURE.md` and `docs/LIMITATIONS.md` before claiming full semantic coverage on arbitrary production C++.
