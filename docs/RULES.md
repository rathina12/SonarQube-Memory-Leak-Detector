# Rule Catalog

| ID | Meaning | Default severity | Confidence notes |
|---|---|---:|---|
| ML001 | Live allocation reaches function exit | Critical | High for local raw pointers |
| ML002 | Pointer overwrite loses final known alias | Critical | High |
| ML003 | Early-return leak | Critical | High |
| ML004 | Conditional path can return with live allocation | Critical | High/medium depending on frontend |
| ML005 | Unmodelled cross-function ownership/escape risk | Major | Reserved for enhanced interprocedural mode |
| ML006 | Double release | Critical | High |
| ML007 | Allocator/deallocator family mismatch | Critical | High |
| ML008 | Direct assignment from `realloc` can lose original pointer | Critical | High |
| ML009 | Use after free/delete | Critical | High |
| ML010 | Allocation inside loop requires per-iteration release proof | Major | Heuristic |

## Suppressing legitimate ownership transfer

Add custom ownership sinks to `config/mlpca.yml`. Example:

```yaml
ownership_sinks:
  - take_ownership
```

Then `take_ownership(p)` transitions the tracked allocation to `Escaped` instead of reporting it as a local leak.
