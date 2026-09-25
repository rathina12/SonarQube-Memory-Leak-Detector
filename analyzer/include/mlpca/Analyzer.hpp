#pragma once
#include "mlpca/Config.hpp"
#include "mlpca/Issue.hpp"
#include "mlpca/Model.hpp"
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace mlpca {

struct AnalysisResult {
  std::vector<Issue> issues;
  Metrics metrics;
};

class Analyzer {
 public:
  explicit Analyzer(AnalyzerConfig config = {});
  AnalysisResult analyzePath(const std::filesystem::path& root);
  AnalysisResult analyzeSource(const std::string& source, const std::string& virtualFile);

 private:
  AnalyzerConfig config_;
};

} // namespace mlpca
