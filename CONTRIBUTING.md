# Contributing

This project welcomes contributions to the C/C++ analyzer, SonarQube integration, benchmarks, tests, and documentation.

## Before changing analyzer logic
Create a minimal reproducible fixture and identify the expected rule. Analyzer changes should be validated against both positive leak cases and safe-code cases to avoid regressions.

## Validation
- Build the analyzer with CMake and run `ctest`.
- Build the SonarQube plugin with Maven.
- Run relevant benchmark fixtures for performance-sensitive changes.

## Pull request checklist
- [ ] Reproduction or fixture is included.
- [ ] Tests cover the fix or feature.
- [ ] False-positive impact was checked.
- [ ] Output remains deterministic.
- [ ] Performance-sensitive changes include before/after evidence.
