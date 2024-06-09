#pragma once
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CodeGen.h>

#include <map>
#include <string>

#include "frontend/ast/ast.h"
#include "visitor/IRInstructionGen.h"

// forward declare llvm types to avoid including llvm headers
namespace llvm {
class Value;
}  // namespace llvm

namespace frontend {
class CodeGenerator {
 public:
  CodeGenerator();
  void generate_code(const Program& p, const std::string& filename);

 private:
  llvm::LLVMContext context_;
  llvm::Module module_;
  llvm::IRBuilder<> builder_;

  static void generate_llvm_ir(const ast::FunctionPtr& f,
                               IRInstructionGen& irgen);
  std::map<const ast::Variable*, llvm::Value*> function_setup(
      const ast::FunctionPtr& f);
  void llvm_verify_generated_ir() const;
  void llvm_optim_pass();
  void llvm_codegen_pass(const std::string& filename,
                         llvm::CodeGenFileType file_type);
  void setup_function_args(
      std::map<const ast::Variable*, llvm::Value*>& allocated_variables,
      llvm::Argument* llvm_arg, const ast::ConstValuePtr& var);
};

}  // namespace frontend
