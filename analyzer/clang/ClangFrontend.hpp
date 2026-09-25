#pragma once
#ifdef MLPCA_WITH_CLANG
#include <string>
namespace mlpca::clang_frontend {
int run(const std::string& compilationDatabaseDir, const std::string& sourcePath);
}
#endif
