#include "frontend/ast.h"
#include "frontend/code_generator.h"
#include "frontend/parse/parser.h"
#include "frontend/visitor/ApplyTypesBuilder.h"
#include "frontend/visitor/DumpAST.h"

int main(int argc, char** argv) {
  // TODO: parse command line arguments
  [[maybe_unused]] bool enable_code_generator;
  if (true) {
    std::string input_fname = "../../test.program";
    if (argc > 1) {
      input_fname = argv[1];
    }
    frontend::Program p = frontend::parse_file(input_fname.c_str());
    frontend::DumpAST dump_ast;
    frontend::ApplyTypesBuilder builder;
    p = builder.build_program(p);
    dump_ast.dump_program(p);
    //    exit(0);
    frontend::CodeGenerator cg;
    std::string output_fname = "output.o";
    if (argc > 2) {
      output_fname = argv[2];
    }
    cg.generate_code(p, output_fname);
  }

  return 0;
}
