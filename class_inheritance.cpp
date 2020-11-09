#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/MultiplexConsumer.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class ClassInheritanceConsumer : public ASTConsumer {
  CompilerInstance& CI;
  std::error_code ec;
  llvm::raw_fd_ostream os;
  std::string nsName;
  PrintingPolicy pp;
public:
  ClassInheritanceConsumer(CompilerInstance& CI, std::string foutName, std::string nsName): CI(CI), os(foutName, ec, llvm::sys::fs::OpenFlags::OF_Append), nsName(nsName), pp(CI.getLangOpts()) {
    pp.SuppressScope = 0;
    pp.FullyQualifiedName = 1;
    pp.PrintCanonicalTypes = 1;
    pp.SuppressTagKeyword = 1;
  }

  bool isInMainFile(SourceLocation loc) {
    SourceManager& mgr = CI.getSourceManager();
    return mgr.getMainFileID() == mgr.getFileID(loc);
  }

  std::string getFullyQualifiedName(QualType type) {
    std::string result = type.getAsString(pp);
    return result;
  }

  std::string getFullyQualifiedName(const Type* type) {
    QualType qt = type->getCanonicalTypeUnqualified();
    return getFullyQualifiedName(qt);
  }

  void HandleTagDeclDefinition(TagDecl* TD) override {
      if (!isInMainFile(TD->getLocation())) {
        return;
      }
      if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(TD)) {
        if (!RD->hasDefinition()) {
          return;
        }
        if (!RD->getNumBases()) {
          return;
        }
        std::string name = getFullyQualifiedName(TD->getTypeForDecl());
        if (name.substr(0, nsName.size()) != nsName) {
          return;
        }
        os << name << ' ';
        for (auto itor = RD->bases_begin(), itor_end = RD->bases_end(); itor != itor_end; ++itor) {
          QualType baseType = itor->getType();
          os << getFullyQualifiedName(baseType) << ' ';
        }
        os << '\n';
      }
    return;
  }
  bool shouldSkipFunctionBody(Decl*) override {
    return true;
  }
};

class PrintClassInheritanceAction : public PluginASTAction {
  std::string foutName;
  std::string nsName;
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {
    return std::make_unique<ClassInheritanceConsumer>(CI, foutName, nsName);
  }
  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    if (args.size() != 2) {
      llvm::errs() << "wrong number of argument, expected one";
      return false;
    }
    foutName = args[0];
    nsName = args[1];
    return true;
  }
};

}

static FrontendPluginRegistry::Add<PrintClassInheritanceAction>
X("print-inh", "print class inheritance");
