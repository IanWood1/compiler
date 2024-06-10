#include <llvm/Support/CommandLine.h>
#include "frontend/ast/ast.h"
#include "frontend/code_generator.h"
#include "frontend/parse/parser.h"
#include "frontend/visitor/ApplyTypesBuilder.h"
#include "frontend/visitor/DumpAST.h"

#include <string>

bool enableDebug;

int main(int argc, char** argv) {

  llvm::cl::opt<std::string> output_filename(
      "o", llvm::cl::desc("Specify output filename"),
      llvm::cl::value_desc("filename"));
  llvm::cl::opt<std::string> input_filename(
      "i", llvm::cl::desc("Specify desc filename"),
      llvm::cl::value_desc("filename"));
  llvm::cl::opt<bool, true> debug("d", llvm::cl::desc("Print debug output"),
                                  llvm::cl::Hidden,
                                  llvm::cl::location(enableDebug));
  llvm::cl::ParseCommandLineOptions(argc, argv);

  frontend::Program p = frontend::parse_file(input_filename.c_str());
  frontend::DumpAST dump_ast;
  frontend::ApplyTypesBuilder builder;
  p = builder.build_program(p);
  if (enableDebug) {
    dump_ast.dump_program(p);
  }
  frontend::CodeGenerator cg;
  cg.generate_code(p, output_filename);

  return 0;
}
