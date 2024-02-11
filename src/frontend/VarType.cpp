#include "frontend/VarType.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <optional>
#include "frontend/diagnostic/debug.h"

namespace frontend {

ConstVarTypePtr VarType::get_array_type(const std::string& type_name,
                                        int64_t n_dims, int64_t n_size,
                                        ConstVarTypePtr& elem_type) {
  TypeIdentifier type_identifier(type_name, n_dims, n_size, TypeCat::ARRAY,
                                 ValCat::NONE);
  return find_var_type_or_create(type_identifier,
                                 MemberTypes{std::move(elem_type)}, {});
}
ConstVarTypePtr VarType::get_atomic_type(const std::string& type_name) {
  TypeCat category;

  if (type_name == "void") {
    category = TypeCat::VOID;
  } else if (type_name == "int64") {
    category = TypeCat::INTEGER;
  } else {
    FRONTEND_ERROR("no matching category for type!");
  }
  TypeIdentifier type_identifier(type_name, NonArrayDim, NonArraySize, category,
                                 ValCat::NONE);

  return find_var_type_or_create(type_identifier, {}, {});
}

ConstVarTypePtr VarType::find_type_by_name(const std::string& type_name) {
  TypeCat category;
  if (type_name == "void") {
    category = TypeCat::VOID;
  } else if (type_name == "int64") {
    category = TypeCat::INTEGER;
  } else {
    category = TypeCat::STRUCTURE;
  }
  TypeIdentifier type_identifier(type_name, NonArrayDim, NonArraySize, category,
                                 ValCat::NONE);

  return find_var_type_or_create(type_identifier, {}, {});
}

ConstVarTypePtr VarType::get_struct_type(
    const std::string& type_name, const MemberTypes& member_types,
    const MemberNameToIndex& member_name_to_index) {
  TypeCat category = TypeCat::STRUCTURE;
  TypeIdentifier type_identifier(type_name, NonArrayDim, NonArraySize, category,
                                 ValCat::NONE);

  return find_var_type_or_create(type_identifier, member_types,
                                 member_name_to_index);
}

ConstVarTypePtr VarType::get_literal_type(const std::string& type_name) {
  TypeIdentifier type_identifier(type_name, NonArrayDim, NonArraySize,
                                 TypeCat::INTEGER, ValCat::PR_VALUE);

  return find_var_type_or_create(type_identifier, {}, {});
}

ConstVarTypePtr VarType::get_ref_type_from() const {
  TypeIdentifier type_identifier(this->type_id_.type_name + "&", NonArrayDim,
                                 NonArraySize, TypeCat::REFERENCE,
                                 ValCat::NONE);
  auto res = find_var_type_or_create(
      type_identifier,
      MemberTypes{find_var_type_or_create(this->type_id_, this->members_,
                                          this->member_name_to_index_)},
      {});
  return res;
}
ConstVarTypePtr VarType::get_pr_value_from() const {
  TypeIdentifier type_identifier = this->type_id_;
  type_identifier.value_category = ValCat::PR_VALUE;
  return find_var_type_or_create(type_identifier, this->members_,
                                 this->member_name_to_index_);
}
ConstVarTypePtr VarType::get_l_value_from() const {
  TypeIdentifier type_identifier = this->type_id_;
  type_identifier.value_category = ValCat::L_VALUE;
  return find_var_type_or_create(type_identifier, this->members_,
                                 this->member_name_to_index_);
}

ConstVarTypePtr VarType::get_x_value_from() const {
  TypeIdentifier type_identifier = this->type_id_;
  type_identifier.value_category = ValCat::X_VALUE;
  return find_var_type_or_create(type_identifier, this->members_,
                                 this->member_name_to_index_);
}

llvm::Type* VarType::get_llvm_in_reg_type(llvm::LLVMContext& context) const {
  // TODO: rename this func
  switch (type_id_.type_category) {
    case TypeCat::REFERENCE:
      ASSERT(!get_referenced_type()->is_ref(), "ref cant ref a ref");
      return get_referenced_type()->get_llvm_in_reg_type(context);
    case TypeCat::ARRAY:
    case TypeCat::STRUCTURE:
      return llvm::PointerType::get(context, DefaultAddressSpace);
    case TypeCat::INTEGER:
      return llvm::Type::getInt64Ty(context);
    case TypeCat::VOID:
      return llvm::Type::getVoidTy(context);
    default:
      FRONTEND_ERROR("unknown type");
  }
}

llvm::Type* VarType::get_llvm_stack_alloc_ty(llvm::LLVMContext& context) const {
  // TODO: rename this func
  std::vector<llvm::Type*> llvm_members(members_.size());
  switch (type_id_.type_category) {
    case TypeCat::REFERENCE:
      return llvm::PointerType::get(context, DefaultAddressSpace);
    case TypeCat::ARRAY:
      return llvm::ArrayType::get(
          get_elem_type()->get_llvm_in_reg_type(context), type_id_.size);
    case TypeCat::INTEGER:
      return llvm::Type::getInt64Ty(context);
    case TypeCat::VOID:
      return llvm::Type::getVoidTy(context);
    case TypeCat::STRUCTURE:
      // todo: refactor to func
      std::transform(members_.begin(), members_.end(), llvm_members.begin(),
                     [&](const ConstVarTypePtr& ptr) {
                       return ptr->get_llvm_stack_alloc_ty(context);
                     });
      return llvm::StructType::create(llvm_members, "test", false);
    default:
      FRONTEND_ERROR("unknown type");
  }
}

int64_t VarType::get_object_size() const {
  switch (type_id_.type_category) {
    case TypeCat::STRUCTURE:
      return get_struct_size();
    case TypeCat::ARRAY:
      return type_id_.size * get_elem_type()->get_object_size();
    case TypeCat::INTEGER:
    case TypeCat::VOID:
    case TypeCat::REFERENCE:
      return 8;
    default:
      FRONTEND_ERROR("unknown type");
  }
}
int64_t VarType::get_struct_size() const {
  int64_t total_size = 0;
  for (auto& member_type : members_) {
    total_size += member_type->get_object_size();
  }
  return total_size;
}

const ConstVarTypePtr& VarType::get_elem_type() const {
  ASSERT(is_array() || (is_ref() || get_referenced_type()->is_array()),
         "type is not of array type");
  if (is_array()) {
    return members_.back();
  } else {
    return get_referenced_type()->members_.back();
  }
}

const VarType& VarType::get_member_type(const std::string& member_name) const {
  ASSERT(is_struct(), "type is not of struct type");
  return *members_[member_name_to_index_.at(member_name)];
}

std::string VarType::get_type_name() const {
  std::string value_category;
  switch (type_id_.value_category) {
    case ValCat::PR_VALUE:
      value_category = "(PR Value)";
      break;
    case ValCat::L_VALUE:
      value_category = "(L Value)";
      break;
    case ValCat::X_VALUE:
      value_category = "(X Value)";
      break;
    case ValCat::NONE:
      value_category = "(NONE)";
      break;
    default:
      FRONTEND_ERROR("no matching category found");
  }
  return type_id_.type_name + " " + value_category;
}

bool VarType::is_array() const {
  return type_id_.type_category == TypeCat::ARRAY;
}

bool VarType::is_struct() const {
  return type_id_.type_category == TypeCat::STRUCTURE;
}

bool VarType::is_ref() const {
  return type_id_.type_category == TypeCat::REFERENCE;
}

bool VarType::is_primitive() const {
  return is_int() || is_void();
}

bool VarType::is_int() const {
  return type_id_.type_category == TypeCat::INTEGER;
}

bool VarType::is_void() const {
  return type_id_.type_category == TypeCat::VOID;
}

ConstVarTypePtr VarType::get_referenced_type() const {
  ASSERT(is_ref(), "getting referenced type of non ref");
  //  ASSERT(members_.size() == 1, "reference should have 1 member");
  return members_.back();
}

bool VarType::is_pr_value() const {
  return type_id_.value_category == ValCat::PR_VALUE;
}

VarType::TypeIdentifier::TypeIdentifier(std::string type_name, int64_t n_dims,
                                        int64_t n_size, TypeCat type_category,
                                        ValCat value_category)
    : n_dims(n_dims),
      size(n_size),
      type_name(std::move(type_name)),
      type_category(type_category),
      value_category(value_category) {}

VarType::VarType(TypeIdentifier type_identifier, MemberTypes members,
                 MemberNameToIndex member_name_to_index)
    : type_id_(std::move(type_identifier)),
      members_(std::move(members)),
      member_name_to_index_(std::move(member_name_to_index)) {}

ConstVarTypePtr VarType::find_var_type_or_create(
    const TypeIdentifier& type_identifier, MemberTypes members,
    MemberNameToIndex member_name_to_index) {

  static std::vector<ConstVarTypePtr> allocated_var_types = {
      // primitive types
      ConstVarTypePtr(new VarType(
          VarType::TypeIdentifier("int64", NonArrayDim, NonArraySize,
                                  TypeCat::INTEGER, ValCat::NONE),
          {}, {})),
      ConstVarTypePtr(
          new VarType(VarType::TypeIdentifier("void", NonArrayDim, NonArraySize,
                                              TypeCat::VOID, ValCat::NONE),
                      {}, {}))};

  // try to find if it exists in the map
  for (auto& type : allocated_var_types) {
    if (type->type_id_ == type_identifier) {
      return type;
    }
  }
  allocated_var_types.emplace_back(new VarType(
      type_identifier, std::move(members), std::move(member_name_to_index)));
  return allocated_var_types.back();
}
VarType::ValCat VarType::get_value_category() const {
  return type_id_.value_category;
}
VarType::TypeCat VarType::get_type_category() const {
  return type_id_.type_category;
}
bool VarType::is_stack() const {
  return is_array() || is_struct();
}
bool VarType::is_object() const {
  return is_array() || is_struct();
}
}  // namespace frontend
