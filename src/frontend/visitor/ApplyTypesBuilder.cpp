//
// Created by Ian Wood on 1/15/2024.
//

#include "frontend/visitor/ApplyTypesBuilder.h"
#include "frontend/types/VarType.h"

namespace frontend {
Program ApplyTypesBuilder::build_program(const Program& program) {
  return traverse_program(program);
}

ast::ConstValuePtr ApplyTypesBuilder::visit_val(
    const ast::Variable& var, TraverseAst::TraversalState& state) {
  auto new_var = ast::Variable::get(var.name, state.new_function);
  if (new_var->type == nullptr)
    new_var->type = var.type;
  return new_var;
}

ast::ConstValuePtr ApplyTypesBuilder::visit_val(const ast::Integer& num,
                                                TraverseAst::TraversalState&) {
  return std::make_shared<ast::Integer>(num.value);
}

ast::ConstValuePtr ApplyTypesBuilder::visit_val(
    const ast::FunctionName& func_name, TraverseAst::TraversalState&) {
  // todo: maybe functions dont always return prvalues
  auto new_func = std::make_shared<ast::FunctionName>(
      std::string(func_name.name), func_name.return_type->get_pr_value_from());
  new_func->type = new_func->return_type;
  return new_func;
}

ast::ConstValuePtr ApplyTypesBuilder::visit_val(
    const ast::BinaryOperation& bin_op, TraverseAst::TraversalState&) {
  auto new_bin_op = std::make_shared<ast::BinaryOperation>(
      bin_op.op, get(*bin_op.lhs), get(*bin_op.rhs));
  // todo: work needed in the type class to support
  if (new_bin_op->lhs->type->is_ref()) {
    // binop dereferences
    new_bin_op->type =
        new_bin_op->lhs->type->get_referenced_type()->get_pr_value_from();
  } else {
    new_bin_op->type = new_bin_op->lhs->type->get_pr_value_from();
  }
  return new_bin_op;
}
ast::ConstValuePtr ApplyTypesBuilder::visit_val(const ast::FunctionCall& call,
                                                TraverseAst::TraversalState&) {
  auto new_call = std::make_unique<ast::FunctionCall>(
      get(*call.function), std::vector<ast::ConstValuePtr>{});
  for (const auto& arg : call.args) {
    new_call->args.push_back(get(*arg));
  }
  new_call->arg_types = call.arg_types;

  // todo: only handle pr value returns
  // careful when changing!!!!, load value expects uses the fact that references are returned as rvalues
  // so that it knows not to load a second time (since they must be loaded by the return statement)
  new_call->type = new_call->function->type->get_pr_value_from();
  return new_call;
}
ast::ConstValuePtr ApplyTypesBuilder::visit_val(const ast::ArrayAccess& access,
                                                TraverseAst::TraversalState&) {
  ASSERT(access.indices.size() == 1, "Only 1d array supported");
  std::vector<ast::ConstValuePtr> indices;
  indices.push_back(get(*access.indices.back()));
  auto new_access = std::make_shared<ast::ArrayAccess>(
      std::move(get(*access.var)), std::move(indices), 1);
  // todo: doesnt support difference between references and nonref
  new_access->type =
      new_access->var->type->get_elem_type()->get_ref_type_from();
  return new_access;
}
ast::ConstValuePtr ApplyTypesBuilder::visit_val(const ast::ArrayAllocate& alloc,
                                                TraverseAst::TraversalState&) {
  auto elem = get(*alloc.elem_value);
  auto elem_type = elem->type;
  auto array_type = VarType::get_array_type(
      elem->type->get_type_name(), 1,
      dynamic_cast<const ast::Integer&>(*alloc.length).value, elem_type);
  auto new_alloc =
      std::make_shared<ast::ArrayAllocate>(get(*alloc.length), std::move(elem));
  new_alloc->type = array_type;
  return new_alloc;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionReturn& ret, TraverseAst::TraversalState&) {
  auto new_ret = std::make_unique<ast::InstructionReturn>();
  if (ret.val != nullptr) {
    new_ret->val = get(*ret.val);
    new_ret->type = new_ret->val->type;
  } else {
    new_ret->type = VarType::get_atomic_type("void");
  }
  return new_ret;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionAssignment& assign, TraverseAst::TraversalState&) {
  auto new_src = get(*assign.src);
  auto new_dst = get(*assign.dst);
  auto new_assign = std::make_unique<ast::InstructionAssignment>(
      std::move(new_dst), std::move(new_src));
  return new_assign;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionFunctionCall& call, TraverseAst::TraversalState&) {
  auto new_call =
      std::make_unique<ast::InstructionFunctionCall>(get(*call.function_call));
  // TODO: could warn about unused result???

  new_call->type = call.function_call->type;
  return new_call;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionWhileLoop& loop, TraverseAst::TraversalState&) {
  auto new_loop = std::make_unique<ast::InstructionWhileLoop>(get(*loop.cond),
                                                              get(*loop.body));
  return new_loop;  // no type associated
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionIfStatement& if_stmt, TraverseAst::TraversalState&) {
  auto new_if_stmt = std::make_unique<ast::InstructionIfStatement>(
      get(*if_stmt.cond), get(*if_stmt.true_scope));
  return new_if_stmt;  // no type associated
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(const ast::InstructionBreak&,
                                                 TraverseAst::TraversalState&) {
  NOT_IMPLEMENTED();
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionContinue&, TraverseAst::TraversalState&) {
  NOT_IMPLEMENTED();
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionDecl& decl, TraverseAst::TraversalState&) {
  auto new_decl =
      std::make_unique<ast::InstructionDecl>(std::vector<ast::ValuePtr>{});
  for (const auto& var : decl.variables) {
    new_decl->variables.push_back(get(*var));
  }
  return new_decl;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(const ast::Scope& scope,
                                                 TraverseAst::TraversalState&) {
  auto new_scope = std::make_unique<ast::Scope>();
  new_scope->type = VarType::get_atomic_type("void");
  for (const auto& inst : scope.instructions) {
    new_scope->instructions.push_back(get(*inst));
  }
  return new_scope;
}
}  // namespace frontend
