#pragma once
#include "mlpca/Analyzer.hpp"
#include <string>

namespace mlpca {
std::string toJson(const AnalysisResult& result, const std::string& projectRoot);
bool writeJsonReport(const AnalysisResult& result, const std::string& projectRoot, const std::string& path);
} // namespace mlpca
