# Project Status

Version: **1.0.0**

## Production-ready components in this repository

- C++ analyzer executable and lifecycle engine
- deterministic JSON output
- rules ML001–ML010 model (ML005 reserved for enhanced cross-function escape diagnostics)
- alias tracking by allocation object
- returned-ownership summaries
- custom memory APIs / ownership sinks
- Java SonarQube external-issue importer
- Linux + Windows build scripts
- regression fixtures
- CI workflow
- sanitizer testing instructions/results

## Deliberately not overclaimed

The dependency-light parser is not a complete ISO C++ semantic engine. The optional Clang LibTooling adapter exists for compiler-grade AST integration, but LLVM development libraries were unavailable in the validation container. See `LIMITATIONS.md` before presenting accuracy or compiler-semantic claims.
