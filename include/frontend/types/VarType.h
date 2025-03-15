#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// forward declare llvm types to avoid including llvm headers
namespace llvm {
class Type;
class LLVMContext;
}  // namespace llvm

namespace frontend {
class VarType;
using ConstVarTypePtr = std::shared_ptr<const VarType>;
class VarType {
 private:
  struct TypeIdentifier;

 public:
  using MemberTypes = std::vector<ConstVarTypePtr>;
  using MemberNameToIndex = std::unordered_map<std::string, int64_t>;
  static constexpr int64_t kNonArrayDim = -1;
  static constexpr int64_t kNonArraySize = -1;

  //
  // static methods for creating/getting types
  //
  static ConstVarTypePtr getArrayType(const std::string& type_name,
                                      int64_t n_dims, int64_t n_size,
                                      ConstVarTypePtr& elem_type);
  static ConstVarTypePtr getAtomicType(const std::string& type_name);
  static ConstVarTypePtr getStructType(
      const std::string& type_name, const MemberTypes& member_types,
      const MemberNameToIndex& member_name_to_index);

  static ConstVarTypePtr getLiteralType(const std::string& type_name);

  static ConstVarTypePtr findTypeByName(const std::string& type_name);

  //
  // get types from other types
  //
  [[nodiscard]] ConstVarTypePtr getRefTypeFrom() const;
  [[nodiscard]] ConstVarTypePtr getPrValueFrom() const;
  [[nodiscard]] ConstVarTypePtr getLValueFrom() const;
  [[nodiscard]] ConstVarTypePtr getXValueFrom() const;

  // non-static
  VarType() = delete;
  ~VarType() = default;

  /// returns the llvm type corresponding to the variable loaded into register
  ///
  ///
  llvm::Type* getLlvmInRegType(llvm::LLVMContext& context) const;

  /// returns the llvm type corresponding to the variable in memory (stack)
  llvm::Type* getLlvmStackAllocTy(llvm::LLVMContext& context) const;

  [[nodiscard]] int64_t getObjectSize() const;

  /// returns the element type of an array type
  [[nodiscard]] const ConstVarTypePtr& getElemType() const;

  /// returns the type that reference points to
  [[nodiscard]] ConstVarTypePtr getReferencedType() const;

  /* @brief returns the type of a member of a struct type
   *
   * @param member_name the name of the member
   */
  [[nodiscard]] const VarType& getMemberType(
      const std::string& member_name) const;

  /* @brief returns the gets the name of the type
   *
   */
  [[nodiscard]] std::string getTypeName() const;
  [[nodiscard]] bool isStack() const;
  [[nodiscard]] bool isObject() const;

  [[nodiscard]] bool isArray() const;
  [[nodiscard]] bool isStruct() const;
  [[nodiscard]] bool isRef() const;
  [[nodiscard]] bool isPrimitive() const;
  [[nodiscard]] bool isInt() const;
  [[nodiscard]] bool isVoid() const;
  [[nodiscard]] bool isPrValue() const;

  enum class ValCat {
    NONE,
    LValue,
    XValue,
    PrValue,
  };
  enum class TypeCat {
    NONE,
    VOID,
    INTEGER,
    REFERENCE,
    ARRAY,
    STRUCTURE,
  };

  // Get Category
  [[nodiscard]] ValCat getValueCategory() const;
  [[nodiscard]] TypeCat getTypeCategory() const;

 private:
  // TODO(ian): TypeIdentifier is unnecessary, type_name should uniquely identify the type via a mangled string
  struct TypeIdentifier {

    explicit TypeIdentifier(std::string type_name,
                            int64_t n_dims = VarType::kNonArrayDim,
                            int64_t n_size = VarType::kNonArraySize,
                            TypeCat type_category = TypeCat::NONE,
                            ValCat value_category = ValCat::NONE);
    // general
    std::string type_name;

    // array specific
    int64_t n_dims;
    int64_t size;

    TypeCat type_category;
    ValCat value_category;

    auto operator<=>(const TypeIdentifier&) const = default;
  };

  // general
  TypeIdentifier type_id_;

  // specific to complex types
  MemberTypes members_;
  MemberNameToIndex member_name_to_index_;

  static constexpr unsigned int kDefaultAddressSpace = 0;

 public:
  VarType(const VarType&) = delete;
  VarType(VarType&&) = delete;

 protected:
  explicit VarType(TypeIdentifier type_identifier, MemberTypes members,
                   MemberNameToIndex member_name_to_index);

  static ConstVarTypePtr findVarTypeOrCreate(
      const TypeIdentifier& type_identifier, MemberTypes members,
      MemberNameToIndex member_name_to_index);
  [[nodiscard]] int64_t getStructSize() const;
};
}  // namespace frontend
