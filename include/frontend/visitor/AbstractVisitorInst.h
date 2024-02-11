#pragma once

#include "../ast.h"
namespace frontend {
// required forward declarations to avoid circular dependencies
namespace ast {
struct Instruction;
struct InstructionReturn;
struct InstructionAssignment;
struct InstructionFunctionCall;
struct InstructionWhileLoop;
struct InstructionIfStatement;
struct InstructionBreak;
struct InstructionContinue;
struct InstructionDecl;
struct Scope;

}  // namespace ast

class AbstractVisitorInst {
 public:
  // ========== Instructions ==========
  virtual void visit(const ast::InstructionReturn* ret) = 0;
  virtual void visit(const ast::InstructionAssignment* assign) = 0;
  virtual void visit(const ast::InstructionFunctionCall* call) = 0;
  virtual void visit(const ast::InstructionWhileLoop* loop) = 0;
  virtual void visit(const ast::InstructionIfStatement* if_stmt) = 0;
  virtual void visit(const ast::InstructionBreak* brk) = 0;
  virtual void visit(const ast::InstructionContinue* cont) = 0;
  virtual void visit(const ast::InstructionDecl* decl) = 0;

  // ========== Scope ==========
  virtual void visit(const ast::Scope* scope) = 0;
};

}  // namespace frontend