#pragma once
#include <map>
#include <vector>
#include "AbstractVisitorInst.h"
#include "AbstractVisitorValue.h"
#include "frontend/ast/ast.h"

namespace frontend {
// Used during ast transformations since ast should be immutable
template <typename Derived, typename InstRetType, typename ValRetType>
class TraverseAst : public AbstractVisitorValue, public AbstractVisitorInst {

 protected:
  struct TraversalState {
    Program* new_program = nullptr;
    const Program* old_program = nullptr;
    ast::Function* new_function{};
    const ast::Function* old_function{};
    std::vector<const ast::Value*> old_value_stack;
    std::vector<const ast::Instruction*> old_instruction_stack;
  };
  TraversalState state_;

  // can be implemented by derived to provide different functionality
  Program traverse_program(const Program& program) {
    Program ret_program;
    state_.new_program = &ret_program;
    state_.old_program = &program;
    for (const auto& func : program.functions) {
      ret_program.functions.push_back(traverse_function(*func));
    }
    return ret_program;
  }

  // can be implemented by derived to provide different functionality
  ast::FunctionPtr traverse_function(const ast::Function& function) {
    ast::FunctionPtr ret_function = std::make_unique<ast::Function>();
    state_.new_function = ret_function.get();
    state_.old_function = &function;
    ret_function->name = std::string(function.name);
    ret_function->scope = get(*function.scope);
    ret_function->type = function.type;
    for (const auto& arg : function.args) {
      ret_function->args.push_back(get(*arg));
    }
    return ret_function;
  }

  decltype(auto) get(const ast::Value& val) {
    state_.old_value_stack.push_back(&val);
    val_ret_ = ValRetType();
    val.accept(this);
    return val_ret_;
  }
  decltype(auto) get(const ast::Instruction& inst) {
    state_.old_instruction_stack.push_back(&inst);
    inst_ret_ = InstRetType();
    inst.accept(this);
    return std::move(inst_ret_);
  }

 private:
  void visit(const ast::Variable* var) override {
    val_ret_ = derived().visit_val(*var, state_);
  }

  void visit(const ast::Integer* num) override {
    val_ret_ = derived().visit_val(*num, state_);
  }

  void visit(const ast::FunctionName* func_name) override {
    val_ret_ = derived().visit_val(*func_name, state_);
  }

  void visit(const ast::BinaryOperation* bin_op) override {
    val_ret_ = derived().visit_val(*bin_op, state_);
  }

  void visit(const ast::FunctionCall* call) override {
    val_ret_ = derived().visit_val(*call, state_);
  }

  void visit(const ast::ArrayAccess* access) override {
    val_ret_ = derived().visit_val(*access, state_);
  }

  void visit(const ast::ArrayAllocate* alloc) override {
    val_ret_ = derived().visit_val(*alloc, state_);
  }

  // ========== Instructions ==========

  void visit(const ast::InstructionReturn* ret) override {
    inst_ret_ = derived().visit_inst(*ret, state_);
  }

  void visit(const ast::InstructionAssignment* assign) override {
    inst_ret_ = derived().visit_inst(*assign, state_);
  }

  void visit(const ast::InstructionFunctionCall* call) override {
    inst_ret_ = derived().visit_inst(*call, state_);
  }

  void visit(const ast::InstructionWhileLoop* loop) override {
    inst_ret_ = derived().visit_inst(*loop, state_);
  }

  void visit(const ast::InstructionIfStatement* if_stmt) override {
    inst_ret_ = derived().visit_inst(*if_stmt, state_);
  }

  void visit(const ast::InstructionBreak* brk) override {
    inst_ret_ = derived().visit_inst(*brk, state_);
  }

  void visit(const ast::InstructionContinue* cont) override {
    inst_ret_ = derived().visit_inst(*cont, state_);
  }

  void visit(const ast::InstructionDecl* decl) override {
    inst_ret_ = derived().visit_inst(*decl, state_);
  }

  // ========== Scope ==========

  void visit(const ast::Scope* scope) override {
    inst_ret_ = derived().visit_inst(*scope, state_);
  }

 private:
  Derived& derived() { return *static_cast<Derived*>(this); }
  ValRetType val_ret_;
  InstRetType inst_ret_;
};

}  // namespace frontend
