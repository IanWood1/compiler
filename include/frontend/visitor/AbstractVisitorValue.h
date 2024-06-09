#pragma once
namespace frontend {
// required forward declarations to avoid circular dependencies
namespace ast {
struct Variable;
struct Integer;
struct FunctionName;
struct BinaryOperation;
struct FunctionCall;
struct ArrayAccess;
struct ArrayAllocate;
}  // namespace ast

class AbstractVisitorValue {
 public:
  // ===========================================
  // define all the visit functions on classes from ast.h here
  // ===========================================

  // ========== Items ==========
  virtual void visit(const ast::Variable* var) = 0;
  virtual void visit(const ast::Integer* num) = 0;
  virtual void visit(const ast::FunctionName* func_name) = 0;
  virtual void visit(const ast::BinaryOperation* bin_op) = 0;
  virtual void visit(const ast::FunctionCall* call) = 0;
  virtual void visit(const ast::ArrayAccess* access) = 0;
  virtual void visit(const ast::ArrayAllocate* alloc) = 0;
};

}  // namespace frontend
