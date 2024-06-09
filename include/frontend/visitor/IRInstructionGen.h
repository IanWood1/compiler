#pragma once

#include <map>

#include "frontend/ast/ast.h"
#include "frontend/visitor/AbstractVisitorInst.h"
#include "frontend/visitor/IRValueGen.h"

// forward declare llvm types to avoid including llvm headers
namespace llvm {
class Type;
class LLVMContext;
class Value;
class Module;
class ConstantFolder;
class IRBuilderDefaultInserter;

template <typename FolderTy, typename InserterTy>
class IRBuilder;

}  // namespace llvm

namespace frontend {

class IRInstructionGen : public AbstractVisitorInst {
 public:
  IRInstructionGen(llvm::IRBuilder<llvm::ConstantFolder,
                                   llvm::IRBuilderDefaultInserter>& builder,
                   llvm::LLVMContext& context, llvm::Module& module,
                   std::map<const ast::Variable*, llvm::Value*>& vars);
  llvm::LLVMContext& context_;
  llvm::Module& module_;
  llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>&
      builder_;
  std::map<const ast::Variable*, llvm::Value*>& allocated_variables_;
  IRValueGen value_gen_;

  llvm::Value* get(const ast::Instruction& i);
  void visit(const ast::InstructionReturn* r) override;
  void visit(const ast::InstructionAssignment* a) override;
  void visit(const ast::InstructionFunctionCall* f) override;
  void visit(const ast::InstructionWhileLoop* w) override;
  void visit(const ast::InstructionIfStatement* f) override;
  void visit(const ast::InstructionBreak* b) override;
  void visit(const ast::InstructionContinue* c) override;
  void visit(const ast::InstructionDecl* v) override;
  void visit(const ast::Scope* s) override;
  llvm::Value* value_ = nullptr;
};
}  // namespace frontend
