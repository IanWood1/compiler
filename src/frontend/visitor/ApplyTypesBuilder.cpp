//
// Created by Ian Wood on 1/15/2024.
//

#include "frontend/visitor/ApplyTypesBuilder.h"
#include <cassert>
#include "frontend/diagnostic/debug.h"
#include "frontend/types/VarType.h"

namespace frontend {
Program ApplyTypesBuilder::build_program(const Program& program) {
  return traverse_program(program);
}

ast::ConstValuePtr ApplyTypesBuilder::visit_val(
    const ast::Variable& var, TraverseAst::TraversalState& state) {
  auto newVar = ast::Variable::get(var.name, state.new_function);
  if (newVar->type == nullptr)
    newVar->type = var.type;
  return newVar;
}

ast::ConstValuePtr ApplyTypesBuilder::visit_val(const ast::Integer& num,
                                                TraverseAst::TraversalState&) {
  return std::make_shared<ast::Integer>(num.value);
}

ast::ConstValuePtr ApplyTypesBuilder::visit_val(
    const ast::FunctionName& func_name, TraverseAst::TraversalState&) {
  // todo: maybe functions dont always return prvalues
  auto newFunc = std::make_shared<ast::FunctionName>(
      std::string(func_name.name), func_name.return_type->getPrValueFrom());
  newFunc->type = newFunc->return_type;
  return newFunc;
}

ast::ConstValuePtr ApplyTypesBuilder::visit_val(
    const ast::BinaryOperation& bin_op, TraverseAst::TraversalState&) {
  auto newBinOp = std::make_shared<ast::BinaryOperation>(
      bin_op.op, get(*bin_op.lhs), get(*bin_op.rhs));
  // todo: work needed in the type class to support
  if (newBinOp->lhs->type->isRef()) {
    // binop dereferences
    newBinOp->type = newBinOp->lhs->type->getReferencedType()->getPrValueFrom();
  } else {
    newBinOp->type = newBinOp->lhs->type->getPrValueFrom();
  }
  return newBinOp;
}
ast::ConstValuePtr ApplyTypesBuilder::visit_val(const ast::FunctionCall& call,
                                                TraverseAst::TraversalState&) {
  auto newCall = std::make_unique<ast::FunctionCall>(
      get(*call.function), std::vector<ast::ConstValuePtr>{});
  for (const auto& arg : call.args) {
    newCall->args.push_back(get(*arg));
  }
  newCall->arg_types = call.arg_types;

  // todo: only handle pr value returns
  // careful when changing!!!!, load value expects uses the fact that references are returned as rvalues
  // so that it knows not to load a second time (since they must be loaded by the return statement)
  newCall->type = newCall->function->type->getPrValueFrom();
  return newCall;
}
ast::ConstValuePtr ApplyTypesBuilder::visit_val(const ast::ArrayAccess& access,
                                                TraverseAst::TraversalState&) {
  assert(access.indices.size() == 1 && "Only 1d array supported");
  std::vector<ast::ConstValuePtr> indices;
  indices.push_back(get(*access.indices.back()));
  auto newAccess = std::make_shared<ast::ArrayAccess>(
      std::move(get(*access.var)), std::move(indices), 1);
  // todo: doesnt support difference between references and nonref
  newAccess->type = newAccess->var->type->getElemType()->getRefTypeFrom();
  return newAccess;
}
ast::ConstValuePtr ApplyTypesBuilder::visit_val(const ast::ArrayAllocate& alloc,
                                                TraverseAst::TraversalState&) {
  auto elem = get(*alloc.elem_value);
  auto elemType = elem->type;
  auto arrayType = VarType::getArrayType(
      elem->type->getTypeName(), 1,
      dynamic_cast<const ast::Integer&>(*alloc.length).value, elemType);
  auto newAlloc =
      std::make_shared<ast::ArrayAllocate>(get(*alloc.length), std::move(elem));
  newAlloc->type = arrayType;
  return newAlloc;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionReturn& ret, TraverseAst::TraversalState&) {
  auto newRet = std::make_unique<ast::InstructionReturn>();
  if (ret.val != nullptr) {
    newRet->val = get(*ret.val);
    newRet->type = newRet->val->type;
  } else {
    newRet->type = VarType::getAtomicType("void");
  }
  return newRet;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionAssignment& assign, TraverseAst::TraversalState&) {
  auto newSrc = get(*assign.src);
  auto newDst = get(*assign.dst);
  auto newAssign = std::make_unique<ast::InstructionAssignment>(
      std::move(newDst), std::move(newSrc));
  return newAssign;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionFunctionCall& call, TraverseAst::TraversalState&) {
  auto newCall =
      std::make_unique<ast::InstructionFunctionCall>(get(*call.function_call));
  // TODO(ian): could warn about unused result???

  newCall->type = call.function_call->type;
  return newCall;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionWhileLoop& loop, TraverseAst::TraversalState&) {
  auto newLoop = std::make_unique<ast::InstructionWhileLoop>(get(*loop.cond),
                                                             get(*loop.body));
  return newLoop;  // no type associated
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(
    const ast::InstructionIfStatement& if_stmt, TraverseAst::TraversalState&) {
  auto newIfStmt = std::make_unique<ast::InstructionIfStatement>(
      get(*if_stmt.cond), get(*if_stmt.true_scope));
  return newIfStmt;  // no type associated
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
  auto newDecl =
      std::make_unique<ast::InstructionDecl>(std::vector<ast::ValuePtr>{});
  for (const auto& var : decl.variables) {
    newDecl->variables.push_back(get(*var));
  }
  return newDecl;
}
ast::ConstInstrPtr ApplyTypesBuilder::visit_inst(const ast::Scope& scope,
                                                 TraverseAst::TraversalState&) {
  auto newScope = std::make_unique<ast::Scope>();
  newScope->type = VarType::getAtomicType("void");
  for (const auto& inst : scope.instructions) {
    newScope->instructions.push_back(get(*inst));
  }
  return newScope;
}
}  // namespace frontend
