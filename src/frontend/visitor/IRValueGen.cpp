#include "frontend/visitor/IRValueGen.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <map>
#include <vector>

#include "frontend/VarType.h"
#include "frontend/ast.h"
#include "frontend/diagnostic/debug.h"

namespace frontend {
namespace {
int64_t get_value_of_integer(const ast::Value* val) {
  const auto* i = dynamic_cast<const ast::Integer*>(val);
  ASSERT(i != nullptr, "expected Integer but didnt receive one");
  return i->value;
}
}  // namespace

IRValueGen::IRValueGen(llvm::IRBuilder<>& builder, llvm::LLVMContext& context,
                       llvm::Module& module,
                       std::map<const ast::Variable*, llvm::Value*>& vars)
    : builder_(builder), context_(context), module_(module), vars_(vars) {}

llvm::Value* IRValueGen::get_loaded_val(const ast::Value* value) {
  llvm::Value* llvm_val = get_val(value);
  const VarType& value_type = *value->type;
  if (value_type.is_pr_value()) {
    return llvm_val;  // already in virtual register
  } else if (value_type.is_int() && !value_type.is_pr_value()) {
    return builder_.CreateLoad(value_type.get_llvm_in_reg_type(context_),
                               llvm_val);
  } else if (value_type.is_array() || value_type.is_struct()) {
    return llvm_val;
  } else if (value_type.is_ref()) {
    return builder_.CreateLoad(value_type.get_llvm_in_reg_type(context_),
                               llvm_val);
  } else if (value_type.is_void()) {
    return nullptr;
  }
  FRONTEND_ERROR("no matching type found for load");
}

llvm::Value* IRValueGen::get_val(const ast::Value* value) {
  ASSERT(value_ == nullptr, "value should be null: overwriting previous value");
  value_ = nullptr;
  value->accept(this);
  ASSERT(value_ != nullptr, "IRValueGen: value is null");
  llvm::Value* llvm_val = value_;
  value_ = nullptr;
  return llvm_val;
}

void IRValueGen::visit(const ast::Variable* v) {
  if (vars_.find(v) == vars_.end()) {
    // pointers to where value in var is located
    llvm::Function* f = builder_.GetInsertBlock()->getParent();
    llvm::IRBuilder<> entry_builder_tmp(&f->getEntryBlock(),
                                        f->getEntryBlock().begin());
    llvm::Value* var = entry_builder_tmp.CreateAlloca(
        v->type->get_llvm_stack_alloc_ty(context_), nullptr, v->name);

    if (v->type->is_ref()) {
      ASSERT(v->type->get_object_size() == 8,
             "Doesnt support != 8 byte primitives or refs");
      builder_.CreateStore(
          llvm::ConstantPointerNull::get(
              v->type->get_llvm_stack_alloc_ty(context_)->getPointerTo(0)),
          var);
    } else if (v->type->is_int()) {
      ASSERT(v->type->get_object_size() == 8,
             "Doesnt support != 8 byte primitives or refs");
      builder_.CreateStore(
          llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(context_), 0),
          var);
    }
    //      builder_.CreateMemSet(
    //          var, llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(context_), 0),
    //          v->type->get_object_size(), llvm::MaybeAlign());

    (vars_)[v] = var;
  }
  value_ = vars_[v];
}

void IRValueGen::visit(const ast::Integer* n) {
  value_ = llvm::ConstantInt::getSigned(n->type->get_llvm_in_reg_type(context_),
                                        n->value);
}
void IRValueGen::visit(const ast::FunctionName* b) {
  NOT_IMPLEMENTED();
}
void IRValueGen::visit(const ast::BinaryOperation* b) {
  switch (b->op) {
    case ast::BinOpId::ADD:
      value_ = builder_.CreateAdd(get_loaded_val(b->lhs.get()),
                                  get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::SUB:
      value_ = builder_.CreateSub(get_loaded_val(b->lhs.get()),
                                  get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::MUL:
      value_ = builder_.CreateMul(get_loaded_val(b->lhs.get()),
                                  get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::AND:
      value_ = builder_.CreateAnd(get_loaded_val(b->lhs.get()),
                                  get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::LT:
      value_ = builder_.CreateICmpSLT(get_loaded_val(b->lhs.get()),
                                      get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::GT:
      value_ = builder_.CreateICmpSGT(get_loaded_val(b->lhs.get()),
                                      get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::EQ:
      value_ = builder_.CreateICmpEQ(get_loaded_val(b->lhs.get()),
                                     get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::SHL:
      value_ = builder_.CreateShl(get_loaded_val(b->lhs.get()),
                                  get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::SHR:
      value_ = builder_.CreateLShr(get_loaded_val(b->lhs.get()),
                                   get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::LEQ:
      value_ = builder_.CreateICmpSLE(get_loaded_val(b->lhs.get()),
                                      get_loaded_val(b->rhs.get()));
      return;
    case ast::BinOpId::GEQ:
      value_ = builder_.CreateICmpSGE(get_loaded_val(b->lhs.get()),
                                      get_loaded_val(b->rhs.get()));
      return;
    default:
      ASSERT(false, "shouldnt reach");
      value_ = nullptr;
      return;
  }
}

void IRValueGen::visit(const ast::FunctionCall* f) {
  const auto* b = dynamic_cast<const ast::FunctionName*>(f->function.get());
  auto* func = module_.getFunction(b->name);
  if (!func) {
    FRONTEND_ERROR("callee function not found");
  }
  std::vector<llvm::Value*> args;
  for (int i = 0; i < f->args.size(); i++) {
    auto& arg = f->args[i];
    auto& expected_type = f->arg_types[i];
    if (expected_type->is_ref()) {
      // pass the reference by value, no need to load
      args.push_back(get_val(arg.get()));
    } else {
      args.push_back(get_loaded_val(arg.get()));
    }
  }
  if (f->type->is_array() || f->type->is_struct()) {
    // return stack obj by value, create a new obj in this frame and pass ptr to last argument
    llvm::Function* llvm_func = builder_.GetInsertBlock()->getParent();
    llvm::IRBuilder<> entry_builder_tmp(&llvm_func->getEntryBlock(),
                                        llvm_func->getEntryBlock().begin());
    llvm::Value* llvm_object_ptr = entry_builder_tmp.CreateAlloca(
        f->type->get_llvm_stack_alloc_ty(context_), nullptr);
    args.push_back(llvm_object_ptr);
  }
  value_ = builder_.CreateCall(static_cast<llvm::Function*>(func), args);
}
void IRValueGen::visit(const ast::ArrayAccess* a) {
  const VarType& variable_type = *a->var->type;
  const VarType& array_type = variable_type.is_ref()
                                  ? *variable_type.get_referenced_type()
                                  : variable_type;
  llvm::Value* base = get_loaded_val(a->var.get());

  // calculate offset (from list of indices)
  std::vector<llvm::Value*> indices = {builder_.getInt64(0)};
  for (auto& index : a->indices) {
    indices.push_back(get_loaded_val(index.get()));
  }
  value_ = builder_.CreateGEP(array_type.get_llvm_stack_alloc_ty(context_),
                              base, indices);
}

void IRValueGen::visit(const ast::ArrayAllocate* a) {
  llvm::Type* llvm_elem_type =
      a->type->get_elem_type()->get_llvm_in_reg_type(context_);

  uint64_t size = get_value_of_integer(a->length.get());

  llvm::Function* f = builder_.GetInsertBlock()->getParent();
  llvm::Type* llvm_arr_type = llvm::ArrayType::get(llvm_elem_type, size);

  llvm::IRBuilder<> entry_builder_tmp(&f->getEntryBlock(),
                                      f->getEntryBlock().begin());
  llvm::Value* llvm_array_ptr =
      entry_builder_tmp.CreateAlloca(llvm_arr_type, nullptr);

  for (uint64_t i = 0; i < size; i++) {
    llvm::Value* elem_ptr =
        builder_.CreateGEP(llvm_arr_type, llvm_array_ptr,
                           {builder_.getInt64(0), builder_.getInt64(i)});
    builder_.CreateStore(get_loaded_val(a->elem_value.get()), elem_ptr);
  }
  //  builder_.CreateMemSet(
  //      llvm_array_ptr,
  //      llvm::ConstantInt::getSigned(llvm::Type::getInt8Ty(context_), 1), size,
  //      llvm::MaybeAlign());
  value_ = llvm_array_ptr;
}

}  // namespace frontend