# Benchmark Plan

Use the same corpus and compiler flags for every compared analyzer configuration. Record:

- source lines / files
- allocations in ground truth
- true positives
- false positives
- false negatives
- precision / recall
- wall-clock analysis time
- peak RSS
- branch/path states created
- path states merged

Do not claim a performance improvement until the CSV contains measured runs.

Suggested comparisons:

1. MLPCA fallback frontend
2. MLPCA Clang frontend
3. Clang Static Analyzer baseline (`unix.Malloc`) where applicable
4. an intentionally unmerged/full exploration research baseline if implemented for the paper
