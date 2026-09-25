# Architecture

## Goal

MLPCA separates parsing, lifecycle analysis and SonarQube integration so that a Sonar API change cannot break the core analyzer.

```text
C/C++ source
    |
    +--> default deterministic frontend ----+
    |                                       |
    +--> optional Clang LibTooling frontend-+--> lifecycle model --> issue engine --> JSON
                                                                        |
                                                                        v
                                                              SonarQube Java Sensor
                                                                        |
                                                                        v
                                                                External Issues UI
```

## Core memory model

An allocation is an object independent of pointer variable names. A set of aliases can reference the same allocation. This prevents the classic false positive where `q = p; free(q);` leaves `p` looking live to a variable-only tracker.

Allocation states:

- `Allocated`
- `Freed`
- `Escaped`
- `Returned`
- `Leaked`
- `Unknown`

Allocation families are tracked (`malloc/calloc/realloc`, `new`, `new[]`, custom) so release-family mismatches are detectable.

## Partial call-path strategy

The analyzer avoids treating every syntactic branch as a completely independent whole-program execution. It retains memory-relevant branch information and records state creation/merge metrics. The enhanced Clang frontend is the integration point for canonical AST/CFG events and function summaries. The dependency-light frontend is kept as a deterministic fallback for builds, CI and demonstrations where LLVM development libraries are unavailable.

## SonarQube adapter

The Java plugin imports findings as external issues through `SensorContext.newExternalIssue()`. It deliberately does **not** register another C/C++ language because doing so can conflict with an existing CFamily analyzer. Report paths are configured with `sonar.mlpca.reportPaths`.

## Report schema

```json
{
  "schemaVersion": 1,
  "engine": "mlpca",
  "projectRoot": "/project",
  "metrics": {},
  "issues": [
    {
      "ruleId": "ML001",
      "severity": "CRITICAL",
      "file": "src/a.c",
      "line": 10,
      "column": 3,
      "message": "...",
      "symbol": "p",
      "allocationKind": "malloc",
      "confidence": 0.98,
      "flow": []
    }
  ]
}
```

The schema has an explicit version so future fields can be introduced without silently corrupting plugin imports.
