# Limitations and Correctness Boundaries

No static analyzer can truthfully promise zero false positives/false negatives for arbitrary C/C++ programs. This project therefore distinguishes **definite/high-confidence findings** from heuristic findings and documents unsupported semantics.

## Dependency-light frontend

The default frontend is deterministic and dependency-light. It is designed for the project rules and test corpus, not as a replacement for a C++ compiler parser. Complex macro expansion, template instantiation, overloaded operators, exception unwinding, virtual dispatch, function-pointer target sets, multi-thread ownership transfer, custom arenas and exotic compiler extensions can require the Clang frontend or explicit ownership configuration.

## Clang mode

`MLPCA_WITH_CLANG=ON` requires compatible LLVM/Clang development packages and a compilation database (`compile_commands.json`). The repository keeps the Clang adapter isolated so LLVM version changes do not affect the tested core.

## Ownership

- A pointer returned from a function is treated as ownership transfer, not a leak inside the callee.
- A configured ownership sink transitions an allocation to `Escaped`.
- Unknown third-party calls are not automatically assumed to free memory.

## `realloc`

`p = realloc(p, n)` is reported because failure can return `NULL` and lose the only reference to the original allocation. The preferred idiom is a temporary pointer followed by a checked assignment.

## Smart pointers / RAII

The fallback frontend focuses on raw ownership. `std::unique_ptr`, `std::shared_ptr`, custom RAII guards, destructors and move semantics should be evaluated with the enhanced Clang semantic frontend. The fallback must not be presented in a paper as complete C++ lifetime proof.

## Research claims

Claims such as “faster than full path analysis” or “higher accuracy” must be backed by benchmark measurements. The repository exposes metrics, but the final paper should report measured results rather than assumed numbers.
