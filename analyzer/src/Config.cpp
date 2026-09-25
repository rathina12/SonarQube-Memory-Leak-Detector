#include "mlpca/Config.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <string>

namespace mlpca {
namespace {
std::string trim(std::string s) {
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}
}

AnalyzerConfig loadConfig(const std::string& path) {
  AnalyzerConfig c;
  if (path.empty()) return c;
  std::ifstream in(path);
  if (!in) return c;

  enum class Section { None, Alloc, Free, Sink };
  Section sec = Section::None;
  std::string line;
  while (std::getline(in, line)) {
    auto pos = line.find('#');
    if (pos != std::string::npos) line = line.substr(0, pos);
    line = trim(line);
    if (line.empty()) continue;
    if (line == "allocators:") { sec = Section::Alloc; continue; }
    if (line == "deallocators:") { sec = Section::Free; continue; }
    if (line == "ownership_sinks:") { sec = Section::Sink; continue; }
    if (line.rfind("- ", 0) == 0) {
      auto v = trim(line.substr(2));
      if (sec == Section::Alloc) c.allocators.insert(v);
      else if (sec == Section::Free) c.deallocators.insert(v);
      else if (sec == Section::Sink) c.ownershipSinks.insert(v);
    }
  }
  return c;
}
} // namespace mlpca
