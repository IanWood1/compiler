#pragma once

#include <llvm/IR/Instructions.h>
#include <map>

#include "frontend/ast/ast.h"
#include "frontend/visitor/AbstractVisitorValue.h"
namespace llvm {
class Type;
class LLVMContext;
class Module;
class Value;
class ConstantFolder;
class IRBuilderDefaultInserter;

template <typename FolderTy, typename InserterTy>
class IRBuilder;

}  // namespace llvm
namespace frontend {
//
class IRValueGen : AbstractVisitorValue {
 public:
  /* @brief Construct a new IRValueGen object
   *
   * @param builder
   * @param context
   * @param module
   * @param vars: a map from frontend::Variable to llvm::Value* (pointer to
   * stack location)
   */
  IRValueGen(llvm::IRBuilder<llvm::ConstantFolder,
                             llvm::IRBuilderDefaultInserter>& builder,
             llvm::LLVMContext& context, llvm::Module& module,
             std::map<const ast::Variable*, llvm::Value*>& vars);

  /* @brief Generates LLVM IR for reading a frontend::Value.
   *
   * @param val: the frontend::value to read
   *
   * @return llvm::Value*: the value read, placed in llvm virtual register
   */
  llvm::Value* get_loaded_val(const ast::Value* val);

  llvm::Value* get_val(const ast::Value* value);

 private:
  llvm::IRBuilder<llvm::ConstantFolder, llvm::IRBuilderDefaultInserter>&
      builder_;
  llvm::LLVMContext& context_;
  llvm::Module& module_;
  std::map<const ast::Variable*, llvm::Value*>& vars_;
  llvm::Value* value_ = nullptr;

  void visit(const ast::Variable* v) override;
  void visit(const ast::Integer* n) override;
  void visit(const ast::FunctionName* b) override;
  void visit(const ast::BinaryOperation* b) override;
  void visit(const ast::FunctionCall* f) override;
  void visit(const ast::ArrayAccess* a) override;
  void visit(const ast::ArrayAllocate* a) override;
};
}  // namespace frontend
