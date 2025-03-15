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
  void generateCode(const Program& p, const std::string& filename);

 private:
  llvm::LLVMContext context_;
  llvm::Module module_;
  llvm::IRBuilder<> builder_;

  static void generateLLVMIR(const ast::FunctionPtr& f,
                             IRInstructionGen& irgen);
  std::map<const ast::Variable*, llvm::Value*> functionSetup(
      const ast::FunctionPtr& f);
  void llvmVerifyGeneratedIr() const;
  void llvmOptimPass();
  void llvmCodegenPass(const std::string& filename,
                       llvm::CodeGenFileType file_type);
  void setupFunctionArgs(
      std::map<const ast::Variable*, llvm::Value*>& allocated_variables,
      llvm::Argument* llvm_arg, const ast::ConstValuePtr& var);
};

}  // namespace frontend
