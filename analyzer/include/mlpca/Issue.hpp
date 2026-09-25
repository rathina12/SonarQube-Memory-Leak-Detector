#pragma once
#include <string>
#include <vector>

namespace mlpca {

enum class Severity { Info, Minor, Major, Critical };

struct FlowStep {
  std::string file;
  int line = 1;
  std::string message;
};

struct Issue {
  std::string ruleId;
  Severity severity = Severity::Major;
  std::string file;
  int line = 1;
  int column = 1;
  std::string message;
  std::string symbol;
  std::string allocationKind;
  double confidence = 1.0;
  std::vector<FlowStep> flow;
};

inline const char* toString(Severity s) {
  switch (s) {
    case Severity::Info: return "INFO";
    case Severity::Minor: return "MINOR";
    case Severity::Major: return "MAJOR";
    case Severity::Critical: return "CRITICAL";
  }
  return "MAJOR";
}

} // namespace mlpca
