#pragma once
#include "TraverseAst.h"
#include "frontend/ast/ast.h"

namespace frontend {

// This class assumes that variables have been already assigned a type
// My attempt at using CRTPish style to make new visitors easier (might have to refactor later)
class ApplyTypesBuilder
    : public TraverseAst<ApplyTypesBuilder, ast::ConstInstrPtr,
                         ast::ConstValuePtr> {
 public:
  Program build_program(const Program& program);

 public:
  ast::ConstValuePtr visit_val(const ast::Variable& var,
                               TraverseAst::TraversalState& state);
  ast::ConstValuePtr visit_val(const ast::Integer& num,
                               TraverseAst::TraversalState& state);
  ast::ConstValuePtr visit_val(const ast::FunctionName& func_name,
                               TraverseAst::TraversalState& state);
  ast::ConstValuePtr visit_val(const ast::BinaryOperation& bin_op,
                               TraverseAst::TraversalState& state);
  ast::ConstValuePtr visit_val(const ast::FunctionCall& call,
                               TraverseAst::TraversalState& state);
  ast::ConstValuePtr visit_val(const ast::ArrayAccess& access,
                               TraverseAst::TraversalState& state);
  ast::ConstValuePtr visit_val(const ast::ArrayAllocate& alloc,
                               TraverseAst::TraversalState& state);

  ast::ConstInstrPtr visit_inst(const ast::InstructionReturn& ret,
                                TraverseAst::TraversalState& state);
  ast::ConstInstrPtr visit_inst(const ast::InstructionAssignment& assign,
                                TraverseAst::TraversalState& state);
  ast::ConstInstrPtr visit_inst(const ast::InstructionFunctionCall& call,
                                TraverseAst::TraversalState& state);
  ast::ConstInstrPtr visit_inst(const ast::InstructionWhileLoop& loop,
                                TraverseAst::TraversalState& state);
  ast::ConstInstrPtr visit_inst(const ast::InstructionIfStatement& if_stmt,
                                TraverseAst::TraversalState& state);
  ast::ConstInstrPtr visit_inst(const ast::InstructionBreak& brk,
                                TraverseAst::TraversalState& state);
  ast::ConstInstrPtr visit_inst(const ast::InstructionContinue& cont,
                                TraverseAst::TraversalState& state);
  ast::ConstInstrPtr visit_inst(const ast::InstructionDecl& decl,
                                TraverseAst::TraversalState& state);
  ast::ConstInstrPtr visit_inst(const ast::Scope& scope,
                                TraverseAst::TraversalState& state);
};

}  // namespace frontend
