#pragma once
#include <cstddef>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace mlpca {

enum class AllocationKind { Malloc, Calloc, Realloc, NewScalar, NewArray, Custom, Unknown };
enum class ReleaseKind { Free, DeleteScalar, DeleteArray, Custom, Unknown };
enum class AllocationState { Allocated, Freed, Escaped, Returned, Leaked, Unknown };

struct SourceLoc {
  std::string file;
  int line = 1;
  int column = 1;
};

struct Allocation {
  std::size_t id = 0;
  AllocationKind kind = AllocationKind::Unknown;
  AllocationState state = AllocationState::Allocated;
  SourceLoc createdAt;
  std::string creator;
  std::set<std::string> aliases;
  std::vector<SourceLoc> releases;
  bool insideLoop = false;
  bool conditional = false;
};

struct Metrics {
  std::size_t filesScanned = 0;
  std::size_t filesFailed = 0;
  std::size_t allocations = 0;
  std::size_t releases = 0;
  std::size_t branchesObserved = 0;
  std::size_t pathStatesCreated = 0;
  std::size_t pathStatesMerged = 0;
  std::size_t issues = 0;
};

inline const char* toString(AllocationKind k) {
  switch (k) {
    case AllocationKind::Malloc: return "malloc";
    case AllocationKind::Calloc: return "calloc";
    case AllocationKind::Realloc: return "realloc";
    case AllocationKind::NewScalar: return "new";
    case AllocationKind::NewArray: return "new[]";
    case AllocationKind::Custom: return "custom";
    case AllocationKind::Unknown: return "unknown";
  }
  return "unknown";
}

} // namespace mlpca
