# Testing Strategy

## Unit / regression tests

`analyzer/tests/test_main.cpp` covers:

1. plain leak
2. safe malloc/free
3. conditional early-return leak
4. alias-safe release
5. pointer overwrite
6. double free
7. new/free mismatch
8. use-after-free
9. realloc misuse
10. interprocedural returned ownership
11. loop allocation risk
12. safe new/delete
13. safe new[]/delete[]
14. direct returned allocation ownership transfer

## Compiler validation

Every fixture in `examples/` should compile under Clang with warnings treated as errors.

## Sanitizers

Build the analyzer tests using AddressSanitizer and UndefinedBehaviorSanitizer. The analyzer test process itself must report no sanitizer faults.

## JSON contract

Every generated report must pass a strict JSON parser before Sonar import.

## Sonar plugin

`sonar-plugin` has report-parser tests and is built in GitHub Actions using Maven. An integration environment should additionally install the JAR into a SonarQube 2026.1+ instance and scan a fixture repository.
