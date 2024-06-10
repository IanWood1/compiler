#include "frontend/visitor/IRInstructionGen.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <map>
#include <vector>

#include <llvm/IR/Metadata.h>
#include "frontend/ast/ast.h"
#include "frontend/diagnostic/debug.h"

namespace frontend {
IRInstructionGen::IRInstructionGen(
    llvm::IRBuilder<>& builder, llvm::LLVMContext& context,
    llvm::Module& module, std::map<const ast::Variable*, llvm::Value*>& vars)
    : builder_(builder),
      context_(context),
      module_(module),
      allocated_variables_(vars),
      value_gen_(builder_, context_, module_, vars) {}

llvm::Value* IRInstructionGen::get(const ast::Instruction& i) {
  i.accept(this);
  return nullptr;
}

void IRInstructionGen::visit(const ast::InstructionReturn* r) {
  if (r->val) {
    if (r->val->type->is_array() || r->val->type->is_struct()) {
      // must copy returned object into the last argument to the function
      llvm::Function* llvm_function = builder_.GetInsertBlock()->getParent();
      llvm::Argument* ret_arg = llvm_function->getArg(
          static_cast<unsigned int>(llvm_function->arg_size() - 1));
      builder_.CreateMemCpy(
          ret_arg, llvm::MaybeAlign(), value_gen_.get_loaded_val(r->val.get()),
          llvm::MaybeAlign(), r->val->type->get_object_size());
      builder_.CreateRet(ret_arg);
      //    } else if (r->val->type->is_ref()) {
      //      // no need to load
      //      builder_.CreateRet(value_gen_.get_val(r->val.get()));
    } else {
      builder_.CreateRet(value_gen_.get_loaded_val(r->val.get()));
    }
  } else {
    builder_.CreateRetVoid();
  }
}

void IRInstructionGen::visit(const ast::InstructionAssignment* a) {
  llvm::Value* llvm_src = value_gen_.get_loaded_val(a->src.get());
  llvm::Value* llvm_dst = value_gen_.get_val(a->dst.get());

  const ConstVarTypePtr& src_type = a->src->type;
  const ConstVarTypePtr& dst_type = a->dst->type;

  bool prim_to_prim = src_type->is_primitive() && src_type->is_primitive();
  bool ref_to_prim = src_type->is_ref() && dst_type->is_primitive();
  bool stack_to_stack = src_type->is_stack() && dst_type->is_stack();
  bool stack_to_ref = src_type->is_stack() && dst_type->is_ref();
  bool ref_to_stack = src_type->is_ref() && dst_type->is_stack();
  bool ref_to_ref = src_type->is_ref() && dst_type->is_ref();

  if (prim_to_prim || ref_to_prim) {
    // just store
    builder_.CreateStore(llvm_src, llvm_dst);
  } else if (ref_to_stack || stack_to_stack) {
    // need to "copy construct" aka just memcpy currently
    builder_.CreateMemCpy(llvm_dst, llvm::MaybeAlign(), llvm_src,
                          llvm::MaybeAlign(), src_type->get_object_size(),
                          false);
  } else if (stack_to_ref || ref_to_ref) {
    // store pointer into ref stack loc
    builder_.CreateStore(llvm_src, llvm_dst);
  } else {
    FRONTEND_ERROR("unknown assignment type");
  }
}
void IRInstructionGen::visit(const ast::InstructionFunctionCall* f) {
  value_gen_.get_loaded_val(f->function_call.get());
}
void IRInstructionGen::visit(const ast::InstructionWhileLoop* w) {
  llvm::Function* the_function = builder_.GetInsertBlock()->getParent();
  llvm::BasicBlock* cond_block =
      llvm::BasicBlock::Create(context_, "while-cond", the_function);
  llvm::BasicBlock* body_block =
      llvm::BasicBlock::Create(context_, "while-body");
  llvm::BasicBlock* continue_block =
      llvm::BasicBlock::Create(context_, "continue");

  builder_.CreateBr(cond_block);
  builder_.SetInsertPoint(cond_block);
  // evaluate expression and compare to 0
  llvm::Value* cond = value_gen_.get_loaded_val(w->cond.get());
  // branch to body or continue
  builder_.CreateCondBr(cond, body_block, continue_block);

  // add body to body_block
  the_function->getBasicBlockList().insert(the_function->end(), body_block);
  builder_.SetInsertPoint(body_block);
  w->body->accept(this);

  // add branch to cond_block
  builder_.CreateBr(cond_block);

  // add continue block
  the_function->getBasicBlockList().insert(the_function->end(), continue_block);
  builder_.SetInsertPoint(continue_block);
}
void IRInstructionGen::visit(const ast::InstructionIfStatement* f) {
  // evaluate expression and compare to 0
  llvm::Value* cond = value_gen_.get_loaded_val(f->cond.get());
  llvm::Function* the_function = builder_.GetInsertBlock()->getParent();
  llvm::BasicBlock* true_block =
      llvm::BasicBlock::Create(context_, "true-block", the_function);
  llvm::BasicBlock* continue_block =
      llvm::BasicBlock::Create(context_, "continue-block");

  builder_.CreateCondBr(cond, true_block, continue_block);

  // true block
  builder_.SetInsertPoint(true_block);
  f->true_scope->accept(this);
  builder_.CreateBr(continue_block);

  // continue block
  the_function->getBasicBlockList().insert(the_function->end(), continue_block);
  builder_.SetInsertPoint(continue_block);
}
void IRInstructionGen::visit(const ast::InstructionBreak* b) {
  ASSERT(false, "not implemented");
}
void IRInstructionGen::visit(const ast::InstructionContinue* c) {
  ASSERT(false, "not implemented");
}
void IRInstructionGen::visit(const ast::InstructionDecl* v) {
  //// allocate in entry for var and add var to allocated_variables_
  std::vector<llvm::Value*> llvm_var(v->variables.size());
  for (int i = 0; i < v->variables.size(); i++) {
    llvm_var[i] = value_gen_.get_val(v->variables[i].get());
  }
}

void IRInstructionGen::visit(const ast::Scope* s) {
  for (const ast::ConstInstrPtr& i : s->instructions) {
    i->accept(this);
  }
}
}  // namespace frontend
