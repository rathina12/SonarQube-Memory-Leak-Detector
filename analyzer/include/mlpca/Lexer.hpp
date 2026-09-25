#pragma once
#include <string>
#include <vector>

namespace mlpca {

struct Token {
  std::string text;
  int line = 1;
  int column = 1;
};

class Lexer {
 public:
  static std::vector<Token> tokenize(const std::string& source);
};

} // namespace mlpca
