#pragma once
#include <set>
#include <string>

namespace mlpca {

struct AnalyzerConfig {
  std::set<std::string> allocators {"malloc", "calloc", "realloc"};
  std::set<std::string> deallocators {"free"};
  std::set<std::string> ownershipSinks;
  std::size_t maxFileBytes = 4 * 1024 * 1024;
  int maxPathDepth = 64;
  bool analyzeHeaders = false;
};

AnalyzerConfig loadConfig(const std::string& path);

} // namespace mlpca
