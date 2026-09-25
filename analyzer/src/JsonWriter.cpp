#include "mlpca/JsonWriter.hpp"
#include <fstream>
#include <sstream>

namespace mlpca {
namespace {
std::string esc(const std::string& s) {
  std::string o;
  o.reserve(s.size() + 16);
  for (char c : s) {
    switch (c) {
      case '\\': o += "\\\\"; break;
      case '"': o += "\\\""; break;
      case '\n': o += "\\n"; break;
      case '\r': o += "\\r"; break;
      case '\t': o += "\\t"; break;
      default: o += c;
    }
  }
  return o;
}
}

std::string toJson(const AnalysisResult& r, const std::string& projectRoot) {
  std::ostringstream o;
  o << "{\n"
    << "  \"schemaVersion\": 1,\n"
    << "  \"engine\": \"mlpca\",\n"
    << "  \"projectRoot\": \"" << esc(projectRoot) << "\",\n";
  o << "  \"metrics\": {"
    << "\"filesScanned\":" << r.metrics.filesScanned << ','
    << "\"filesFailed\":" << r.metrics.filesFailed << ','
    << "\"allocations\":" << r.metrics.allocations << ','
    << "\"releases\":" << r.metrics.releases << ','
    << "\"branchesObserved\":" << r.metrics.branchesObserved << ','
    << "\"pathStatesCreated\":" << r.metrics.pathStatesCreated << ','
    << "\"pathStatesMerged\":" << r.metrics.pathStatesMerged << ','
    << "\"issues\":" << r.metrics.issues << "},\n";
  o << "  \"issues\": [\n";
  for (std::size_t i = 0; i < r.issues.size(); ++i) {
    const auto& x = r.issues[i];
    o << "    {"
      << "\"ruleId\":\"" << esc(x.ruleId) << "\","
      << "\"severity\":\"" << toString(x.severity) << "\","
      << "\"file\":\"" << esc(x.file) << "\","
      << "\"line\":" << x.line << ','
      << "\"column\":" << x.column << ','
      << "\"message\":\"" << esc(x.message) << "\","
      << "\"symbol\":\"" << esc(x.symbol) << "\","
      << "\"allocationKind\":\"" << esc(x.allocationKind) << "\","
      << "\"confidence\":" << x.confidence << ','
      << "\"flow\":[";
    for (std::size_t j = 0; j < x.flow.size(); ++j) {
      const auto& f = x.flow[j];
      if (j) o << ',';
      o << "{\"file\":\"" << esc(f.file) << "\",\"line\":" << f.line << ",\"message\":\"" << esc(f.message) << "\"}";
    }
    o << "]}" << (i + 1 < r.issues.size() ? "," : "") << "\n";
  }
  o << "  ]\n}\n";
  return o.str();
}

bool writeJsonReport(const AnalysisResult& r, const std::string& root, const std::string& path) {
  std::ofstream out(path);
  if (!out) return false;
  out << toJson(r, root);
  return static_cast<bool>(out);
}
} // namespace mlpca
