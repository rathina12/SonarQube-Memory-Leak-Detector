#include "mlpca/Lexer.hpp"
#include <cctype>

namespace mlpca {

std::vector<Token> Lexer::tokenize(const std::string& source) {
  std::vector<Token> out;
  int line = 1;
  int col = 1;
  std::size_t i = 0;
  auto advance = [&](char c) {
    if (c == '\n') { ++line; col = 1; }
    else { ++col; }
  };

  while (i < source.size()) {
    char c = source[i];
    if (std::isspace(static_cast<unsigned char>(c))) { advance(c); ++i; continue; }

    if (c == '/' && i + 1 < source.size() && source[i + 1] == '/') {
      while (i < source.size() && source[i] != '\n') { advance(source[i]); ++i; }
      continue;
    }
    if (c == '/' && i + 1 < source.size() && source[i + 1] == '*') {
      advance('/'); advance('*'); i += 2;
      while (i + 1 < source.size() && !(source[i] == '*' && source[i + 1] == '/')) { advance(source[i]); ++i; }
      if (i + 1 < source.size()) { advance('*'); advance('/'); i += 2; }
      continue;
    }

    if (c == '"' || c == '\'') {
      const char quote = c;
      const int sl = line, sc = col;
      std::string t;
      t += c; advance(c); ++i;
      while (i < source.size()) {
        char x = source[i]; t += x; advance(x); ++i;
        if (x == '\\' && i < source.size()) { t += source[i]; advance(source[i]); ++i; continue; }
        if (x == quote) break;
      }
      out.push_back({t, sl, sc});
      continue;
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      int sl = line, sc = col;
      std::string t;
      while (i < source.size()) {
        char x = source[i];
        if (!(std::isalnum(static_cast<unsigned char>(x)) || x == '_')) break;
        t += x; advance(x); ++i;
      }
      out.push_back({t, sl, sc});
      continue;
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
      int sl = line, sc = col;
      std::string t;
      while (i < source.size()) {
        char x = source[i];
        if (!(std::isalnum(static_cast<unsigned char>(x)) || x == '.' || x == '_')) break;
        t += x; advance(x); ++i;
      }
      out.push_back({t, sl, sc});
      continue;
    }

    const int sl = line, sc = col;
    const std::string two = i + 1 < source.size() ? source.substr(i, 2) : std::string();
    static const char* ops[] = {"==", "!=", "<=", ">=", "++", "--", "->", "&&", "||", "+=", "-=", "*=", "/=", "::", "<<", ">>"};
    bool matched = false;
    for (auto op : ops) {
      if (two == op) {
        out.push_back({two, sl, sc});
        advance(source[i]); advance(source[i + 1]); i += 2;
        matched = true;
        break;
      }
    }
    if (matched) continue;
    out.push_back({std::string(1, c), sl, sc});
    advance(c); ++i;
  }
  return out;
}

} // namespace mlpca
