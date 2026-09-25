#ifdef MLPCA_WITH_CLANG
#include "ClangFrontend.hpp"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include <memory>

namespace mlpca::clang_frontend {

// This adapter intentionally stays thin: the production analyzer core is independent of
// Clang/SonarQube. It proves the supported integration point and is the place to feed
// canonical AST/CFG events into the same lifecycle engine used by the fallback frontend.
class Visitor : public clang::RecursiveASTVisitor<Visitor> {
 public:
  explicit Visitor(clang::ASTContext& c) : ctx(c) {}
  bool VisitCallExpr(clang::CallExpr* call) {
    if (const auto* callee = call->getDirectCallee()) {
      (void)callee->getNameAsString(); // event hook: allocator/deallocator/function-call
    }
    return true;
  }
  bool VisitCXXNewExpr(clang::CXXNewExpr*) { return true; }   // event hook: new/new[]
  bool VisitCXXDeleteExpr(clang::CXXDeleteExpr*) { return true; } // event hook: delete/delete[]
 private:
  clang::ASTContext& ctx;
};

class Consumer : public clang::ASTConsumer {
 public:
  void HandleTranslationUnit(clang::ASTContext& ctx) override { Visitor(ctx).TraverseDecl(ctx.getTranslationUnitDecl()); }
};
class Action : public clang::ASTFrontendAction {
 public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance&, llvm::StringRef) override {
    return std::make_unique<Consumer>();
  }
};

int run(const std::string& compilationDatabaseDir, const std::string& sourcePath) {
  std::string error;
  auto db = clang::tooling::CompilationDatabase::loadFromDirectory(compilationDatabaseDir, error);
  if (!db) return 2;
  std::vector<std::string> files{sourcePath};
  clang::tooling::ClangTool tool(*db, files);
  return tool.run(clang::tooling::newFrontendActionFactory<Action>().get());
}
} // namespace mlpca::clang_frontend
#endif
