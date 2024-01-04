#pragma once
#include <iostream>
#include "TraverseAst.h"

namespace frontend {
class DumpAST : public AbstractVisitorInst, public AbstractVisitorValue {
 public:
  explicit DumpAST();

  void dump_program(const Program& program);

  void dump_function(const ast::Function& function);

  void dump_instruction(const ast::Instruction& instruction);

  void dump_value(const ast::Value& value);

 private:
  void visit(const ast::Variable* var) override;
  void visit(const ast::Integer* num) override;
  void visit(const ast::FunctionName* func_name) override;
  void visit(const ast::BinaryOperation* bin_op) override;
  void visit(const ast::FunctionCall* call) override;
  void visit(const ast::ArrayAccess* access) override;
  void visit(const ast::ArrayAllocate* alloc) override;

  // ========== Instructions ==========
  void visit(const ast::InstructionReturn* ret) override;
  void visit(const ast::InstructionAssignment* assign) override;
  void visit(const ast::InstructionFunctionCall* call) override;
  void visit(const ast::InstructionWhileLoop* loop) override;
  void visit(const ast::InstructionIfStatement* if_stmt) override;
  void visit(const ast::InstructionBreak* brk) override;
  void visit(const ast::InstructionContinue* cont) override;
  void visit(const ast::InstructionDecl* decl) override;

  // ========== Scope ==========
  void visit(const ast::Scope* scope) override;

  std::ostream& stream_;
  std::string prefix_ = "\n";
};

}  // namespace frontend
