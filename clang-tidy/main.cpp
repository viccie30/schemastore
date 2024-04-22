#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"
#include <clang-tidy/ClangTidy.h>
#include <clang-tidy/ClangTidyCheck.h>
#include <clang-tidy/ClangTidyForceLinker.h>
#include <clang-tidy/ClangTidyModule.h>
#include <clang-tidy/ClangTidyModuleRegistry.h>

using namespace clang::tidy;
using namespace clang;
// using namespace clang::ast_matchers;
// using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

static const char *AnalyzerCheckNamePrefix = "clang-analyzer-";

int main(int argc, char **argv) {
  const bool options = argv[1] == StringRef("options");

  llvm::StringSet<> Options;
  ClangTidyOptions Opts;
  Opts.Checks = "*";
  clang::tidy::ClangTidyContext Context(
      std::make_unique<DefaultOptionsProvider>(ClangTidyGlobalOptions(), Opts),
      true);
  Context.setOptionsCollector(&Options);
  ClangTidyCheckFactories Factories;
  for (const ClangTidyModuleRegistry::entry &Module :
       ClangTidyModuleRegistry::entries()) {
    Module.instantiate()->addCheckFactories(Factories);
  }

  for (const auto &Factory : Factories) {
    if (!options)
      llvm::outs() << Factory.getKey() << '\n';
    Factory.getValue()(Factory.getKey(), &Context);
    if (options)
      for (auto &&option : Options)
        llvm::outs() << option.first() << '\n';
    Options.clear();
  }

  SmallString<64> Buffer(AnalyzerCheckNamePrefix);
  size_t DefSize = Buffer.size();
  if (!options)
    for (const auto &AnalyzerCheck :
         AnalyzerOptions::getRegisteredCheckers(true)) {
      Buffer.truncate(DefSize);
      Buffer.append(AnalyzerCheck);
      llvm::outs() << Buffer << '\n';
    }
}
