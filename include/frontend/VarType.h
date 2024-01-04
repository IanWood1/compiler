#pragma once
#include <cstdint>
#include <memory>
#include <optional>
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
  static constexpr int64_t NonArrayDim = -1;
  static constexpr int64_t NonArraySize = -1;

  //
  // static methods for creating/getting types
  //
  static ConstVarTypePtr get_array_type(const std::string& type_name,
                                        int64_t n_dims, int64_t n_size,
                                        ConstVarTypePtr& elem_type);
  static ConstVarTypePtr get_atomic_type(const std::string& type_name);
  static ConstVarTypePtr get_struct_type(
      const std::string& type_name, const MemberTypes& member_types,
      const MemberNameToIndex& member_name_to_index);

  static ConstVarTypePtr get_literal_type(const std::string& type_name);

  static ConstVarTypePtr find_type_by_name(const std::string& type_name);

  //
  // get types from other types
  //
  ConstVarTypePtr get_ref_type_from() const;
  ConstVarTypePtr get_pr_value_from() const;
  ConstVarTypePtr get_l_value_from() const;
  ConstVarTypePtr get_x_value_from() const;

  // non-static
  VarType() = delete;
  ~VarType() = default;

  /// returns the llvm type corresponding to the variable loaded into register
  ///
  ///
  llvm::Type* get_llvm_in_reg_type(llvm::LLVMContext& context) const;

  /// returns the llvm type corresponding to the variable in memory (stack)
  llvm::Type* get_llvm_stack_alloc_ty(llvm::LLVMContext& context) const;

  int64_t get_object_size() const;

  /// returns the element type of an array type
  [[nodiscard]] const ConstVarTypePtr& get_elem_type() const;

  /// returns the type that reference points to
  [[nodiscard]] ConstVarTypePtr get_referenced_type() const;

  /* @brief returns the type of a member of a struct type
   *
   * @param member_name the name of the member
   */
  [[nodiscard]] const VarType& get_member_type(
      const std::string& member_name) const;

  /* @brief returns the gets the name of the type
   *
   */
  [[nodiscard]] std::string get_type_name() const;
  [[nodiscard]] bool is_stack() const;
  [[nodiscard]] bool is_object() const;

  [[nodiscard]] bool is_array() const;
  [[nodiscard]] bool is_struct() const;
  [[nodiscard]] bool is_ref() const;
  [[nodiscard]] bool is_primitive() const;
  [[nodiscard]] bool is_int() const;
  [[nodiscard]] bool is_void() const;
  [[nodiscard]] bool is_pr_value() const;

  enum class ValCat {
    NONE,
    L_VALUE,
    X_VALUE,
    PR_VALUE,
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
  [[nodiscard]] ValCat get_value_category() const;
  [[nodiscard]] TypeCat get_type_category() const;

 private:
  // TODO: TypeIdentifier is unnecessary, type_name should uniquely identify the type via a mangled string
  struct TypeIdentifier {

    explicit TypeIdentifier(std::string type_name,
                            int64_t n_dims = VarType::NonArrayDim,
                            int64_t n_size = VarType::NonArraySize,
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

  static constexpr unsigned int DefaultAddressSpace = 0;

 public:
  VarType(const VarType&) = delete;
  VarType(VarType&&) = delete;

 protected:
  explicit VarType(TypeIdentifier type_identifier, MemberTypes members,
                   MemberNameToIndex member_name_to_index);

  static ConstVarTypePtr find_var_type_or_create(
      const TypeIdentifier& type_identifier, MemberTypes members,
      MemberNameToIndex member_name_to_index);
  int64_t get_struct_size() const;
};
}  // namespace frontend
