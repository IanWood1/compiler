#include "frontend/parse/parser.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>
#include "frontend/ast/ast.h"
#include "frontend/diagnostic/debug.h"
#include "frontend/types/VarType.h"

#undef FIRING_DEBUG
#ifdef FIRING_DEBUG
#define PEGTL_PRINT_RULE(actionName)                                      \
  std::cout << "At line " << in.position().line                           \
            << (" in code firing rule from action " #actionName) << ":\n" \
            << in.string() << std::endl;                                  \
  bool isScopesNull = false;                                              \
  for (auto& scope : state.parsed_scopes) {                               \
    for (auto& inst : scope->instructions) {                              \
      if (inst == nullptr) {                                              \
        isScopesNull = true;                                              \
      }                                                                   \
    }                                                                     \
  }                                                                       \
  ASSERT(!isScopesNull, "state.parsed_scopes contains nullptr");          \
  bool isItemsNull = false;                                               \
  for (auto& item : state.parsed_items) {                                 \
    if (item == nullptr) {                                                \
      isItemsNull = true;                                                 \
    }                                                                     \
  }                                                                       \
  ASSERT(!isItemsNull, "state.parsed_items contains nullptr");
#define PEGTL_ACTION_FIRE_PRINT(msg) std::cout << msg << std::endl;
#else
#define PEGTL_PRINT_RULE(actionName) (void*)0
#define PEGTL_ACTION_FIRE_PRINT(msg) (void*)0
#endif

// namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
namespace pegtl = TAO_PEGTL_NAMESPACE;
using namespace pegtl;

namespace frontend {
namespace parser {

struct State {
  std::vector<std::unique_ptr<ast::Scope>> parsed_scopes;
  std::vector<ast::ValuePtr> parsed_items;
  std::vector<std::vector<ast::ConstValuePtr>> parsed_function_args;
  std::vector<ast::Value> parsed_defined_function_args;
  std::vector<ast::ValuePtr> parsed_declared_vars;
  ast::BinOpId parsed_binop = ast::BinOpId::NONE;

  // for types
  std::vector<ConstVarTypePtr> parsed_vartypes;
  std::vector<std::string> parsed_type_names;

  // for structs
  std::vector<std::string> parsed_struct_member_names;
  std::string parsed_struct_name;
};

/*
 * Grammar rules from now on.
 */
struct name
    : pegtl::seq<pegtl::plus<pegtl::sor<pegtl::alpha, pegtl::one<'_'>>>,
                 pegtl::star<
                     pegtl::sor<pegtl::alpha, pegtl::one<'_'>, pegtl::digit>>> {
};

/*
 * Keywords.
 */
struct str_return : TAO_PEGTL_STRING("return") {};
struct str_arrow : TAO_PEGTL_STRING("=") {};  //"

struct str_leq : TAO_PEGTL_STRING("<=") {};
struct str_geq : TAO_PEGTL_STRING(">=") {};
struct str_eq : TAO_PEGTL_STRING("==") {};
struct str_lt : TAO_PEGTL_STRING("<") {};
struct str_gt : TAO_PEGTL_STRING(">") {};
struct str_add : TAO_PEGTL_STRING("+") {};
struct str_sub : TAO_PEGTL_STRING("-") {};
struct str_mul : TAO_PEGTL_STRING("*") {};
struct str_shl : TAO_PEGTL_STRING("<<") {};
struct str_shr : TAO_PEGTL_STRING(">>") {};
struct str_and : TAO_PEGTL_STRING("&") {};

struct str_print : TAO_PEGTL_STRING("print") {};
struct str_input : TAO_PEGTL_STRING("input") {};

struct keywords : pegtl::sor<str_return> {};

struct function_name_rule : pegtl::seq<name> {};

struct variable_rule : pegtl::seq<name> {};

struct number
    : pegtl::seq<pegtl::opt<pegtl::sor<pegtl::one<'-'>, pegtl::one<'+'>>>,
                 pegtl::plus<pegtl::digit>> {};

struct function_name_as_label_rule : function_name_rule {};

struct library_function : pegtl::sor<str_print, str_input> {};

struct comment
    : pegtl::disable<TAO_PEGTL_STRING("//"), pegtl::until<pegtl::eolf>> {};

struct seps : pegtl::star<pegtl::sor<pegtl::ascii::space, comment>> {};

struct binop_rule : pegtl::sor<str_leq, str_geq, str_shl, str_shr, str_eq,
                               str_lt, str_gt, str_add, str_sub, str_mul,

                               str_and> {};
// attempt at adding more than 2 operands
struct array_access_rule;
struct function_call_rule;
struct array_allocate_rule;
struct binary_operation_rule;

struct single_expression_rule
    : pegtl::seq<pegtl::sor<
          pegtl::seq<pegtl::at<array_access_rule>, array_access_rule>,
          pegtl::seq<pegtl::at<function_call_rule>, function_call_rule>,
          pegtl::seq<pegtl::at<array_allocate_rule>, array_allocate_rule>,
          variable_rule, number>> {};

struct expression_rule
    : pegtl::seq<seps,
                 pegtl::sor<binary_operation_rule, single_expression_rule>,
                 seps> {};

struct binary_operation_rule
    : pegtl::seq<
          pegtl::seq<
              pegtl::at<pegtl::seq<seps, single_expression_rule, seps,
                                   binop_rule, seps>>,
              pegtl::seq<seps, single_expression_rule, seps, binop_rule, seps>>,
          pegtl::sor<binary_operation_rule, single_expression_rule>, seps> {};

struct type_rule;
struct type_name_rule : pegtl::seq<pegtl::not_at<keywords>, name> {};
struct basic_type_rule
    : pegtl::seq<type_name_rule, seps,
                 pegtl::star<TAO_PEGTL_STRING("<"), seps, type_rule, seps,
                             TAO_PEGTL_STRING(">")>> {};

struct array_type_rule
    : pegtl::seq<basic_type_rule, seps,
                 pegtl::star<TAO_PEGTL_STRING("<"), seps, type_rule, seps,
                             TAO_PEGTL_STRING(">")>,
                 seps,
                 pegtl::plus<TAO_PEGTL_STRING("["), seps, number, seps,
                             TAO_PEGTL_STRING("]")>> {};
struct reference_type_rule
    : pegtl::seq<pegtl::at<pegtl::sor<array_type_rule, basic_type_rule>>,
                 pegtl::sor<array_type_rule, basic_type_rule>, seps,
                 TAO_PEGTL_STRING("&")> {};
struct type_rule
    : pegtl::sor<
          pegtl::seq<pegtl::at<reference_type_rule>, reference_type_rule>,
          pegtl::seq<pegtl::at<array_type_rule>, array_type_rule>,
          basic_type_rule> {};

struct variable_in_declaration_rule : variable_rule {};

struct var_declaration_rule
    : pegtl::seq<seps, type_rule, seps, variable_in_declaration_rule,
                 pegtl::star<seps, pegtl::one<','>, seps,
                             variable_in_declaration_rule>,
                 seps> {};

struct function_argument_rule : pegtl::seq<seps, expression_rule, seps> {};

struct function_definition_argument_rule
    : pegtl::seq<seps, type_rule, seps, variable_rule, seps> {};

struct function_call_name_rule
    : pegtl::seq<seps,
                 pegtl::sor<library_function, function_name_as_label_rule>,
                 seps> {};

struct function_call_rule
    : pegtl::seq<seps, pegtl::sor<function_call_name_rule>, seps,
                 TAO_PEGTL_STRING("("), seps,
                 pegtl::star<function_argument_rule, seps,
                             pegtl::star<TAO_PEGTL_STRING(",")>, seps>,
                 seps, TAO_PEGTL_STRING(")"), seps> {};

struct array_access_rule
    : pegtl::seq<seps, variable_rule, seps,
                 pegtl::plus<pegtl::seq<seps, TAO_PEGTL_STRING("["), seps,
                                        expression_rule, seps,
                                        TAO_PEGTL_STRING("]"), seps>>,
                 seps> {};

struct array_allocate_rule
    : pegtl::seq<seps, TAO_PEGTL_STRING("["), seps,
                 pegtl::seq<expression_rule, seps, TAO_PEGTL_STRING(";"), seps,
                            number>,
                 seps, TAO_PEGTL_STRING("]"), seps> {};

struct Instruction_variable_declaration_rule : var_declaration_rule {};

struct Instruction_return_rule_value
    : pegtl::seq<seps, str_return, seps, expression_rule, seps> {};

struct Instruction_return_rule_void : pegtl::seq<seps, str_return, seps> {};

struct Instruction_assignment_rule
    : pegtl::seq<seps,
                 pegtl::sor<pegtl::seq<pegtl::at<array_access_rule>,
                                       array_access_rule>,
                            variable_rule>,
                 seps, str_arrow, seps, pegtl::must<expression_rule>, seps> {};

struct Instruction_function_call_rule
    : pegtl::seq<seps, function_call_rule, seps> {};
// predeclare Scope_rule
struct Scope_rule;
struct Instruction_while_rule
    : pegtl::seq<seps, TAO_PEGTL_STRING("while"), seps, TAO_PEGTL_STRING("("),
                 seps, binary_operation_rule, seps, TAO_PEGTL_STRING(")"), seps,
                 Scope_rule, seps> {};

struct Instruction_if_rule
    : pegtl::seq<seps, TAO_PEGTL_STRING("if"), seps, TAO_PEGTL_STRING("("),
                 seps, binary_operation_rule, seps, TAO_PEGTL_STRING(")"), seps,
                 Scope_rule, seps> {};

struct Instruction_break_rule
    : pegtl::seq<seps, TAO_PEGTL_STRING("break"), seps> {};

struct Instruction_continue_rule
    : pegtl::seq<seps, TAO_PEGTL_STRING("continue"), seps> {};

struct Instruction_rule
    : pegtl::sor<
          pegtl::seq<pegtl::at<Instruction_while_rule>, Instruction_while_rule>,
          pegtl::seq<pegtl::at<Instruction_if_rule>, Instruction_if_rule>,
          pegtl::seq<pegtl::at<Instruction_break_rule>, Instruction_break_rule>,
          pegtl::seq<pegtl::at<Instruction_continue_rule>,
                     Instruction_continue_rule>,
          pegtl::seq<pegtl::at<Scope_rule>, Scope_rule>,
          pegtl::seq<pegtl::at<Instruction_function_call_rule>,
                     Instruction_function_call_rule>,
          pegtl::seq<pegtl::at<Instruction_assignment_rule>,
                     Instruction_assignment_rule>,
          pegtl::seq<pegtl::at<Instruction_variable_declaration_rule>,
                     Instruction_variable_declaration_rule>,
          pegtl::seq<pegtl::at<Instruction_return_rule_value>,
                     Instruction_return_rule_value>,
          pegtl::seq<pegtl::at<Instruction_return_rule_void>,
                     Instruction_return_rule_void>

          > {};

struct Instructions_rule
    : pegtl::star<pegtl::seq<seps, Instruction_rule, seps>> {};

struct new_scope_rule : TAO_PEGTL_STRING("{") {};

struct end_scope_rule : TAO_PEGTL_STRING("}") {};

struct Scope_rule : pegtl::seq<seps, new_scope_rule, seps, Instructions_rule,
                               seps, end_scope_rule, seps> {};

struct Function_rule
    : pegtl::seq<
          seps, type_rule, seps, function_name_rule, seps, pegtl::one<'('>,
          seps,
          pegtl::star<pegtl::seq<seps, function_definition_argument_rule, seps,
                                 pegtl::star<TAO_PEGTL_STRING(",")>, seps>>,
          pegtl::one<')'>, seps, Scope_rule, seps> {};

struct struct_member_name_rule : pegtl::seq<name> {};
struct struct_definition_member_rule
    : pegtl::seq<seps, type_rule, seps, struct_member_name_rule, seps> {};
struct struct_name_rule : pegtl::seq<name> {};
struct Struct_rule
    : pegtl::seq<TAO_PEGTL_STRING("struct"), seps, struct_name_rule, seps,
                 TAO_PEGTL_STRING("{"), seps,
                 pegtl::star<struct_definition_member_rule>, seps,
                 TAO_PEGTL_STRING("}")> {};

struct Functions_rule
    : pegtl::plus<seps, pegtl::sor<Struct_rule, Function_rule>, seps> {};

struct entry_point_rule : Functions_rule {};

struct grammar : pegtl::must<entry_point_rule> {};

/*
 * Actions attached to grammar rules.
 */
template <typename Rule>
struct action : pegtl::nothing<Rule> {};

template <>
struct action<function_name_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE("function_name_rule");
    auto new_f = std::make_unique<ast::Function>();
    new_f->name = in.string();
    new_f->type = state.parsed_vartypes.back();
    state.parsed_vartypes.pop_back();
    //    if (new_f->return_type->is_array()) {
    //      new_f->return_dim = state.parsed_dims.back();
    //      state.parsed_dims.pop_back();
    //    }
    p.functions.push_back(std::move(new_f));
  }
};

//struct struct_member_name_rule : pegtl::seq<name> {};
//struct struct_definition_member_rule
//    : pegtl::seq<seps, type_rule, seps, struct_member_name_rule, seps> {};
//struct struct_name_rule : pegtl::seq<name> {};
//struct Struct_rule
//    : pegtl::seq<TAO_PEGTL_STRING("struct"), seps, struct_name_rule, seps,
//                 TAO_PEGTL_STRING("{"), seps,
//                 pegtl::star<struct_definition_member_rule>, seps,
//                 TAO_PEGTL_STRING("}")> {};
template <>
struct action<struct_member_name_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(struct_member_name_rule);
    state.parsed_struct_member_names.push_back(in.string());
  }
};
template <>
struct action<struct_definition_member_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(struct_definition_member_rule);
  }
};
template <>
struct action<struct_name_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(struct_name_rule);
    state.parsed_struct_name = in.string();
  }
};
template <>
struct action<Struct_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Struct_rule);
    auto struct_decl = std::make_unique<ast::StructDecl>();
    struct_decl->name = state.parsed_struct_name;
    struct_decl->member_types = std::move(state.parsed_vartypes);
    for (int i = 0; i < state.parsed_struct_member_names.size(); i++) {
      auto& mem_name = state.parsed_struct_member_names[i];
      struct_decl->member_name_to_index[mem_name] = i;
    }
    struct_decl->type =
        VarType::get_struct_type(struct_decl->name, struct_decl->member_types,
                                 struct_decl->member_name_to_index);
    p.structs.push_back(std::move(struct_decl));

    //    VarType::get_struct_type();
  }
};
template <>
struct action<type_name_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(type_name_rule);
    const std::string& type_name = in.string();
    state.parsed_type_names.push_back(type_name);
  }
};

template <>
struct action<basic_type_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(basic_type_rule);
    std::string type_name = in.string();
    type_name.erase(std::remove(type_name.begin(), type_name.end(), ' '),
                    type_name.end());
    state.parsed_vartypes.push_back(VarType::find_type_by_name(type_name));
  }
};

template <>
struct action<array_type_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(array_type_rule);
    auto* size = dynamic_cast<ast::Integer*>(state.parsed_items.back().get());
    ASSERT(size != nullptr, "size of array is not an integer");
    std::string type_name = in.string();
    type_name.erase(std::remove(type_name.begin(), type_name.end(), ' '),
                    type_name.end());
    auto elem_type = std::move(state.parsed_vartypes.back());
    state.parsed_vartypes.pop_back();
    state.parsed_vartypes.emplace_back(
        VarType::get_array_type(type_name, 1, size->value, elem_type));
  }
};

template <>
struct action<reference_type_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(reference_type_rule);
    const auto& referenced_type = state.parsed_vartypes.back();
    auto ref_type = referenced_type->get_ref_type_from();
    state.parsed_vartypes.pop_back();
    state.parsed_vartypes.push_back(ref_type);
  }
};

template <>
struct action<function_name_as_label_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(function_name_as_label_rule);
    auto& current_f = p.functions.back();
    for (auto& f : p.functions) {
      if (f->name == in.string()) {
        state.parsed_items.emplace_back(
            std::make_shared<ast::FunctionName>(in.string(), f->type));
        return;
      }
    }
    FRONTEND_ERROR("could not find called function! " + in.string());
  }
};

template <>
struct action<number> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(number);
    state.parsed_items.push_back(
        std::make_shared<ast::Integer>(std::stoll(in.string())));
  }
};

template <>
struct action<variable_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(variable_rule);
    auto& current_f = p.functions.back();
    auto v = ast::Variable::get(in.string(), current_f.get());
    v->name = in.string();
    state.parsed_items.push_back(std::move(v));
  }
};

template <>
struct action<library_function> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(library_function);
    state.parsed_items.push_back(
        std::make_shared<ast::FunctionName>(in.string(), nullptr));
  }
};

template <>
struct action<function_argument_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(function_argument_rule);
    state.parsed_function_args.back().push_back(
        std::move(state.parsed_items.back()));
    state.parsed_items.pop_back();
  }
};

template <>
struct action<binop_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(binop_rule);
    state.parsed_binop = ast::string_to_binop(in.string());
  }
};

template <>
struct action<binary_operation_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(binary_operation_rule);
    auto& current_f = p.functions.back();
    auto& rhs = state.parsed_items.back();
    auto& lhs = state.parsed_items[state.parsed_items.size() - 2];
    auto binop = std::make_shared<ast::BinaryOperation>(
        state.parsed_binop, std::move(lhs), std::move(rhs));
    state.parsed_items.pop_back();
    state.parsed_items.pop_back();
    state.parsed_items.push_back(std::move(binop));
  }
};

template <>
struct action<array_access_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(array_access_rule);
    std::string str = in.string();
    int64_t num_args = std::count(str.begin(), str.end(), '[');
    auto& current_f = p.functions.back();
    std::vector<ast::ConstValuePtr> indices;
    for (int i = 0; i < num_args; i++) {
      indices.push_back(state.parsed_items.back());
      state.parsed_items.pop_back();
    }
    std::reverse(indices.begin(), indices.end());
    auto& var = state.parsed_items.back();
    auto array_access = std::make_shared<ast::ArrayAccess>(
        std::move(var), std::move(indices), in.position().line);
    state.parsed_items.pop_back();
    state.parsed_items.push_back(std::move(array_access));
  }
};

template <>
struct action<array_allocate_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(array_allocate_rule);
    ast::ConstValuePtr length = std::move(state.parsed_items.back());
    state.parsed_items.pop_back();
    ast::ConstValuePtr init_value = std::move(state.parsed_items.back());
    state.parsed_items.pop_back();
    state.parsed_items.push_back(std::make_shared<ast::ArrayAllocate>(
        std::move(length), std::move(init_value)));
  }
};

template <>
struct action<function_definition_argument_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(function_definition_argument_rule);
    ast::ValuePtr i = std::move(state.parsed_items.back());
    state.parsed_items.pop_back();

    auto var = dynamic_cast<ast::Variable*>(i.get());
    ASSERT(var != nullptr,
           "Expected variable in function definition argument rule");
    var->type = std::move(state.parsed_vartypes.back());
    state.parsed_vartypes.pop_back();

    p.functions.back()->args.push_back(i);
  }
};

template <>
struct action<Instruction_return_rule_value> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_return_rule_value);
    auto& current_f = p.functions.back();
    state.parsed_scopes.back()->instructions.push_back(
        std::make_unique<ast::InstructionReturn>(state.parsed_items.back()));
    state.parsed_items.pop_back();
  }
};

template <>
struct action<Instruction_return_rule_void> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_return_rule_void);
    auto& current_f = p.functions.back();
    state.parsed_scopes.back()->instructions.push_back(
        std::make_unique<ast::InstructionReturn>());
  }
};

template <>
struct action<variable_in_declaration_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(variable_in_declaration_rule);
    auto& current_f = p.functions.back();
    auto var = ast::Variable::get(in.string(), current_f.get());
    state.parsed_declared_vars.push_back(std::move(var));
  }
};

template <>
struct action<Instruction_variable_declaration_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_variable_declaration_rule);
    for (auto& v : state.parsed_declared_vars) {
      auto var = dynamic_cast<ast::Variable*>(v.get());
      var->type =
          state.parsed_vartypes.back()
              ->get_l_value_from();  // need to copy sharedptr to each var
      //      if (var->type->is_array()) {
      //        state.parsed_dims.back();
      //      }
    }
    state.parsed_vartypes.pop_back();
    state.parsed_scopes.back()->instructions.push_back(
        std::make_unique<ast::InstructionDecl>(
            std::move(state.parsed_declared_vars)));
  }
};

template <>
struct action<Instruction_while_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_while_rule);
    auto& current_f = p.functions.back();

    auto& body = state.parsed_scopes.back()->instructions.back();
    auto& cond = state.parsed_items.back();

    auto loop = std::make_unique<ast::InstructionWhileLoop>(std::move(cond),
                                                            std::move(body));
    state.parsed_scopes.back()->instructions.pop_back();
    state.parsed_items.pop_back();
    state.parsed_scopes.back()->instructions.push_back(std::move(loop));
  }
};

template <>
struct action<Instruction_if_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_if_rule);
    auto& current_f = p.functions.back();
    auto i = std::make_unique<ast::InstructionIfStatement>();
    ASSERT(
        dynamic_cast<const ast::Scope*>(
            state.parsed_scopes.back()->instructions.back().get()) != nullptr,
        "Expected scope in if rule");
    i->true_scope = std::move(state.parsed_scopes.back()->instructions.back());
    state.parsed_scopes.back()->instructions.pop_back();
    i->cond = state.parsed_items.back();
    state.parsed_items.pop_back();
    state.parsed_scopes.back()->instructions.push_back(std::move(i));
  }
};

template <>
struct action<Instruction_break_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_break_rule);
    auto& current_f = p.functions.back();
    auto i = std::make_unique<ast::InstructionBreak>();
    state.parsed_scopes.back()->instructions.push_back(std::move(i));
  }
};

template <>
struct action<Instruction_continue_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_continue_rule);
    auto& current_f = p.functions.back();
    auto i = std::make_unique<ast::InstructionContinue>();
    state.parsed_scopes.back()->instructions.push_back(std::move(i));
  }
};

template <>
struct action<Instruction_assignment_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_assignment_rule);
    auto& current_f = p.functions.back();
    auto& src = state.parsed_items.back();
    auto& dst = state.parsed_items[state.parsed_items.size() - 2];
    if (dst == nullptr && src == nullptr) {
      FRONTEND_ERROR("dst and src are nullptr");
    }
    auto i = std::make_unique<ast::InstructionAssignment>(std::move(dst),
                                                          std::move(src));
    state.parsed_items.pop_back();
    state.parsed_items.pop_back();

    // check if i->dst is a variable and has type "code"
    auto* dst_raw = dynamic_cast<const ast::Variable*>(i->dst.get());
    // if (dst_raw != nullptr && dst_raw->type->is_code()) {
    //   // change src to a functionName
    //   auto src = dynamic_cast<const Variable *>(i->src.get());
    //   if (src == nullptr) {
    //     FRONTEND_ERROR("Expected variable in assignment rule");
    //   }
    //   auto fn = std::make_shared<FunctionName>(std::string(src->name),
    //   nullptr); i->src = fn;

    //  // remove fn->name from Variable::variables[current_f]
    //  auto it = Variable::variables[current_f.get()].find(fn->name);
    //  if (it != Variable::variables[current_f.get()].end()) {
    //    Variable::variables[current_f.get()].erase(it);
    //  }
    //}

    state.parsed_scopes.back()->instructions.push_back(std::move(i));
  }
};

template <>
struct action<function_call_name_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(function_call_name_rule);
    state.parsed_function_args.emplace_back();
  }
};

template <>
struct action<function_call_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(function_call_rule);
    auto& current_f = p.functions.back();
    auto f_call = std::make_shared<ast::FunctionCall>(
        std::move(state.parsed_items.back()),
        std::move(state.parsed_function_args.back()));
    auto f_name =
        dynamic_cast<const ast::FunctionName*>(f_call->function.get())->name;
    for (auto& func : p.functions) {
      if (func->name == f_name) {
        for (auto& param : func->args) {
          f_call->arg_types.push_back(param->type);
        }
        break;
      }
    }
    state.parsed_function_args.pop_back();
    state.parsed_items.pop_back();
    state.parsed_items.push_back(std::move(f_call));
  }
};

template <>
struct action<Instruction_function_call_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(Instruction_function_call_rule);
    auto& current_f = p.functions.back();
    state.parsed_scopes.back()->instructions.push_back(
        std::make_unique<ast::InstructionFunctionCall>(
            state.parsed_items.back()));

    state.parsed_items.pop_back();
  }
};

template <>
struct action<new_scope_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(new_scope_rule);
    auto& current_f = p.functions.back();
    state.parsed_scopes.emplace_back(std::make_unique<ast::Scope>());
  }
};

template <>
struct action<end_scope_rule> {
  template <typename Input>
  static void apply(const Input& in, Program& p, State& state) {
    PEGTL_PRINT_RULE(end_scope_rule);
    auto& current_f = p.functions.back();
    if (state.parsed_scopes.size() == 1) {
      current_f->scope = std::move(state.parsed_scopes.back());
      state.parsed_scopes.pop_back();
    } else {
      auto scope = std::move(state.parsed_scopes.back());
      state.parsed_scopes.pop_back();
      state.parsed_scopes.back()->instructions.push_back(std::move(scope));
    }
  }
};

}  // namespace parser

Program parse_file(const char* file_name) {
  /*
   * Check the grammar for some possible issues.
   */
  pegtl::analyze<parser::grammar>();

  /*
   * Parse.
   */
  file_input<> file_input(file_name);
  Program p;
  parser::State state;
  bool ret = parse<parser::grammar, parser::action>(file_input, p, state);
  ASSERT(ret, "parse failed");

  return p;
}
}  // namespace frontend
