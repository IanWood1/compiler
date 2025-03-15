#include "frontend/types/VarType.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>

#include <cstdint>
#include <string>
#include <vector>

#include "frontend/diagnostic/debug.h"

namespace frontend {

ConstVarTypePtr VarType::getArrayType(const std::string& type_name,
                                      int64_t n_dims, int64_t n_size,
                                      ConstVarTypePtr& elem_type) {
  TypeIdentifier typeIdentifier(type_name, n_dims, n_size, TypeCat::ARRAY,
                                ValCat::NONE);
  return findVarTypeOrCreate(typeIdentifier, MemberTypes{std::move(elem_type)},
                             {});
}
ConstVarTypePtr VarType::getAtomicType(const std::string& type_name) {
  TypeCat category;

  if (type_name == "void") {
    category = TypeCat::VOID;
  } else if (type_name == "int64") {
    category = TypeCat::INTEGER;
  } else {
    FRONTEND_ERROR("no matching category for type!");
  }
  TypeIdentifier typeIdentifier(type_name, kNonArrayDim, kNonArraySize,
                                category, ValCat::NONE);

  return findVarTypeOrCreate(typeIdentifier, {}, {});
}

ConstVarTypePtr VarType::findTypeByName(const std::string& type_name) {
  TypeCat category;
  if (type_name == "void") {
    category = TypeCat::VOID;
  } else if (type_name == "int64") {
    category = TypeCat::INTEGER;
  } else {
    category = TypeCat::STRUCTURE;
  }
  TypeIdentifier typeIdentifier(type_name, kNonArrayDim, kNonArraySize,
                                category, ValCat::NONE);

  return findVarTypeOrCreate(typeIdentifier, {}, {});
}

ConstVarTypePtr VarType::getStructType(
    const std::string& type_name, const MemberTypes& member_types,
    const MemberNameToIndex& member_name_to_index) {
  TypeCat category = TypeCat::STRUCTURE;
  TypeIdentifier typeIdentifier(type_name, kNonArrayDim, kNonArraySize,
                                category, ValCat::NONE);

  return findVarTypeOrCreate(typeIdentifier, member_types,
                             member_name_to_index);
}

ConstVarTypePtr VarType::getLiteralType(const std::string& type_name) {
  TypeIdentifier typeIdentifier(type_name, kNonArrayDim, kNonArraySize,
                                TypeCat::INTEGER, ValCat::PrValue);

  return findVarTypeOrCreate(typeIdentifier, {}, {});
}

ConstVarTypePtr VarType::getRefTypeFrom() const {
  TypeIdentifier typeIdentifier(this->type_id_.type_name + "&", kNonArrayDim,
                                kNonArraySize, TypeCat::REFERENCE,
                                ValCat::NONE);
  auto res = findVarTypeOrCreate(
      typeIdentifier,
      MemberTypes{findVarTypeOrCreate(this->type_id_, this->members_,
                                      this->member_name_to_index_)},
      {});
  return res;
}
ConstVarTypePtr VarType::getPrValueFrom() const {
  TypeIdentifier typeIdentifier = this->type_id_;
  typeIdentifier.value_category = ValCat::PrValue;
  return findVarTypeOrCreate(typeIdentifier, this->members_,
                             this->member_name_to_index_);
}
ConstVarTypePtr VarType::getLValueFrom() const {
  TypeIdentifier typeIdentifier = this->type_id_;
  typeIdentifier.value_category = ValCat::LValue;
  return findVarTypeOrCreate(typeIdentifier, this->members_,
                             this->member_name_to_index_);
}

ConstVarTypePtr VarType::getXValueFrom() const {
  TypeIdentifier typeIdentifier = this->type_id_;
  typeIdentifier.value_category = ValCat::XValue;
  return findVarTypeOrCreate(typeIdentifier, this->members_,
                             this->member_name_to_index_);
}

llvm::Type* VarType::getLlvmInRegType(llvm::LLVMContext& context) const {
  // TODO(ian): rename this func
  switch (type_id_.type_category) {
    case TypeCat::REFERENCE:
      ASSERT(!get_referenced_type()->is_ref(), "ref cant ref a ref");
      return getReferencedType()->getLlvmInRegType(context);
    case TypeCat::ARRAY:
    case TypeCat::STRUCTURE:
      return llvm::PointerType::get(context, kDefaultAddressSpace);
    case TypeCat::INTEGER:
      return llvm::Type::getInt64Ty(context);
    case TypeCat::VOID:
      return llvm::Type::getVoidTy(context);
    default:
      FRONTEND_ERROR("unknown type");
  }
}

llvm::Type* VarType::getLlvmStackAllocTy(llvm::LLVMContext& context) const {
  // TODO(): rename this func
  std::vector<llvm::Type*> llvmMembers(members_.size());
  switch (type_id_.type_category) {
    case TypeCat::REFERENCE:
      return llvm::PointerType::get(context, kDefaultAddressSpace);
    case TypeCat::ARRAY:
      return llvm::ArrayType::get(getElemType()->getLlvmInRegType(context),
                                  type_id_.size);
    case TypeCat::INTEGER:
      return llvm::Type::getInt64Ty(context);
    case TypeCat::VOID:
      return llvm::Type::getVoidTy(context);
    case TypeCat::STRUCTURE:
      // todo: refactor to func
      std::transform(members_.begin(), members_.end(), llvmMembers.begin(),
                     [&](const ConstVarTypePtr& ptr) {
                       return ptr->getLlvmStackAllocTy(context);
                     });
      return llvm::StructType::create(llvmMembers, "test", false);
    default:
      FRONTEND_ERROR("unknown type");
  }
}

int64_t VarType::getObjectSize() const {
  switch (type_id_.type_category) {
    case TypeCat::STRUCTURE:
      return getStructSize();
    case TypeCat::ARRAY:
      return type_id_.size * getElemType()->getObjectSize();
    case TypeCat::INTEGER:
    case TypeCat::VOID:
    case TypeCat::REFERENCE:
      return 8;
    default:
      FRONTEND_ERROR("unknown type");
  }
}
int64_t VarType::getStructSize() const {
  int64_t totalSize = 0;
  for (const auto& memberType : members_) {
    totalSize += memberType->getObjectSize();
  }
  return totalSize;
}

const ConstVarTypePtr& VarType::getElemType() const {
  ASSERT(is_array() || (is_ref() || get_referenced_type()->is_array()),
         "type is not of array type");
  if (isArray()) {
    return members_.back();
  }
  return getReferencedType()->members_.back();
}

const VarType& VarType::getMemberType(const std::string& member_name) const {
  ASSERT(is_struct(), "type is not of struct type");
  return *members_[member_name_to_index_.at(member_name)];
}

std::string VarType::getTypeName() const {
  std::string valueCategory;
  switch (type_id_.value_category) {
    case ValCat::PrValue:
      valueCategory = "(PR Value)";
      break;
    case ValCat::LValue:
      valueCategory = "(L Value)";
      break;
    case ValCat::XValue:
      valueCategory = "(X Value)";
      break;
    case ValCat::NONE:
      valueCategory = "(NONE)";
      break;
    default:
      FRONTEND_ERROR("no matching category found");
  }
  return type_id_.type_name + " " + valueCategory;
}

bool VarType::isArray() const {
  return type_id_.type_category == TypeCat::ARRAY;
}

bool VarType::isStruct() const {
  return type_id_.type_category == TypeCat::STRUCTURE;
}

bool VarType::isRef() const {
  return type_id_.type_category == TypeCat::REFERENCE;
}

bool VarType::isPrimitive() const {
  return isInt() || isVoid();
}

bool VarType::isInt() const {
  return type_id_.type_category == TypeCat::INTEGER;
}

bool VarType::isVoid() const {
  return type_id_.type_category == TypeCat::VOID;
}

ConstVarTypePtr VarType::getReferencedType() const {
  ASSERT(is_ref(), "getting referenced type of non ref");
  //  ASSERT(members_.size() == 1, "reference should have 1 member");
  return members_.back();
}

bool VarType::isPrValue() const {
  return type_id_.value_category == ValCat::PrValue;
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

ConstVarTypePtr VarType::findVarTypeOrCreate(
    const TypeIdentifier& type_identifier, MemberTypes members,
    MemberNameToIndex member_name_to_index) {

  static std::vector<ConstVarTypePtr> allocatedVarTypes = {
      // primitive types
      ConstVarTypePtr(new VarType(
          VarType::TypeIdentifier("int64", kNonArrayDim, kNonArraySize,
                                  TypeCat::INTEGER, ValCat::NONE),
          {}, {})),
      ConstVarTypePtr(new VarType(
          VarType::TypeIdentifier("void", kNonArrayDim, kNonArraySize,
                                  TypeCat::VOID, ValCat::NONE),
          {}, {}))};

  // try to find if it exists in the map
  for (auto& type : allocatedVarTypes) {
    if (type->type_id_ == type_identifier) {
      return type;
    }
  }
  allocatedVarTypes.emplace_back(new VarType(
      type_identifier, std::move(members), std::move(member_name_to_index)));
  return allocatedVarTypes.back();
}
VarType::ValCat VarType::getValueCategory() const {
  return type_id_.value_category;
}
VarType::TypeCat VarType::getTypeCategory() const {
  return type_id_.type_category;
}
bool VarType::isStack() const {
  return isArray() || isStruct();
}
bool VarType::isObject() const {
  return isArray() || isStruct();
}
}  // namespace frontend
