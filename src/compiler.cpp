#include <llvm/Support/CommandLine.h>
#include "frontend/ast/ast.h"
#include "frontend/code_generator.h"
#include "frontend/parse/parser.h"
#include "frontend/visitor/ApplyTypesBuilder.h"
#include "frontend/visitor/DumpAST.h"

#include <string>

bool enableDebug;

// Create a custom category for compiler options
static llvm::cl::OptionCategory CompilerCategory("Compiler Options");

int main(int argc, char** argv) {

  llvm::cl::opt<std::string> outputFilename(
      "o", llvm::cl::desc("Specify output filename"),
      llvm::cl::value_desc("filename"), llvm::cl::cat(CompilerCategory));
  llvm::cl::opt<std::string> inputFilename(
      "i", llvm::cl::desc("Specify input filename"),
      llvm::cl::value_desc("filename"), llvm::cl::cat(CompilerCategory));
  llvm::cl::opt<bool, true> debug(
      "d", llvm::cl::desc("Print debug output"), llvm::cl::Hidden,
      llvm::cl::location(enableDebug), llvm::cl::cat(CompilerCategory));

  // Hide all LLVM options and only show compiler options
  llvm::cl::HideUnrelatedOptions(CompilerCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv, "Compiler\n");

  frontend::Program p = frontend::parseFile(inputFilename.c_str());
  frontend::DumpAST dumpAst;
  frontend::ApplyTypesBuilder builder;
  p = builder.build_program(p);
  if (enableDebug) {
    dumpAst.dump_program(p);
  }
  frontend::CodeGenerator cg;
  cg.generateCode(p, outputFilename);

  return 0;
}
