#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "frontend/types/VarType.h"
#include "frontend/visitor/AbstractVisitorInst.h"
#include "frontend/visitor/AbstractVisitorValue.h"

namespace frontend {

namespace ast {
struct Value;
struct Instruction;
struct Function;
struct StructDecl;

using ConstValuePtr = std::shared_ptr<const Value>;
using ValuePtr = std::shared_ptr<Value>;
using ConstInstrPtr = std::unique_ptr<const ast::Instruction>;
using InstrPtr = std::unique_ptr<ast::Instruction>;
using ConstFunctionPtr = std::unique_ptr<const Function>;
using FunctionPtr = std::unique_ptr<Function>;
using StructDeclPtr = std::unique_ptr<StructDecl>;
using ConstStructDeclPtr = std::unique_ptr<const StructDecl>;
}  // namespace ast

struct Program {
 public:
  std::vector<ast::FunctionPtr> functions;
  std::vector<ast::ConstStructDeclPtr> structs;
};

namespace ast {

enum BinOpId { NONE, ADD, MUL, AND, SUB, SHL, SHR, LT, GT, LEQ, GEQ, EQ };
std::string binopToString(BinOpId op);
struct Instruction;
struct Value;
struct Function;
BinOpId stringToBinop(const std::string& op);

// All AST nodes (Function, Program, and Value) inherit from this
struct TypedNode {
  virtual ~TypedNode() = default;
  TypedNode() = default;
  explicit TypedNode(ConstVarTypePtr ptr) : type(std::move(ptr)){};
  ConstVarTypePtr type;
};

struct Value : TypedNode {
 public:
  virtual void accept(AbstractVisitorValue* v) const = 0;

  Value() = default;
  explicit Value(ConstVarTypePtr type_ptr) : TypedNode(std::move(type_ptr)){};
  ~Value() override = 0;
};
struct Variable : public Value {
 public:
  void accept(AbstractVisitorValue* v) const override;
  static std::shared_ptr<Variable> get(const std::string& name,
                                       const Function* f);
  static std::map<const Function*,
                  std::map<std::string, std::shared_ptr<Variable>>>
      variables_;

  std::string name;
  ~Variable() override = default;

 protected:
  Variable() = default;
};
struct Integer : public Value {
 public:
  explicit Integer(int64_t value);
  Integer() = delete;
  ~Integer() override = default;

  void accept(AbstractVisitorValue* v) const override;

  int64_t value;
};
struct BinaryOperation : public Value {
 public:
  BinaryOperation(BinOpId op, ConstValuePtr&& lhs, ConstValuePtr&& rhs);
  BinaryOperation() = delete;
  ~BinaryOperation() override = default;

  void accept(AbstractVisitorValue* v) const override;
  BinOpId op = BinOpId::NONE;
  ConstValuePtr lhs;
  ConstValuePtr rhs;
};
struct FunctionName : public Value {
 public:
  FunctionName(std::string&& name, ConstVarTypePtr ret);
  FunctionName() = delete;
  ~FunctionName() override = default;
  ConstVarTypePtr return_type;
  void accept(AbstractVisitorValue* v) const override;
  std::string name;
};
struct FunctionCall : public Value {
 public:
  FunctionCall(ConstValuePtr&& function, std::vector<ConstValuePtr>&& args);
  FunctionCall() = delete;
  ~FunctionCall() override = default;

  void accept(AbstractVisitorValue* v) const override;
  ConstValuePtr function;
  std::vector<ConstValuePtr> args;
  std::vector<ConstVarTypePtr> arg_types;
};
struct ArrayAccess : public Value {
 public:
  ArrayAccess(ConstValuePtr&& var, std::vector<ConstValuePtr>&& indices,
              uint64_t line_number);
  ArrayAccess() = delete;
  ~ArrayAccess() override = default;

  void accept(AbstractVisitorValue* v) const override;
  ConstValuePtr var;
  std::vector<ConstValuePtr> indices;
};
struct ArrayAllocate : public Value {
 public:
  ArrayAllocate(ConstValuePtr&& length, ConstValuePtr&& elem_value);
  ArrayAllocate() = delete;
  ~ArrayAllocate() override = default;
  void accept(AbstractVisitorValue* v) const override;
  const ConstValuePtr length;
  const ConstValuePtr elem_value;
}; /*
 * Instruction interface.
 */
struct Instruction : TypedNode {
 public:
  virtual void accept(AbstractVisitorInst* v) const = 0;
  ~Instruction() override = default;

}; /*
 * Instructions.
 */
struct InstructionReturn : public Instruction {
 public:
  explicit InstructionReturn(ConstValuePtr&& val);
  InstructionReturn();  // return void
  void accept(AbstractVisitorInst* v) const override;

  ConstValuePtr val;
};
struct InstructionAssignment : public Instruction {
 public:
  InstructionAssignment(ConstValuePtr&& dst, ConstValuePtr&& src);
  InstructionAssignment() = delete;
  void accept(AbstractVisitorInst* v) const override;

  ConstValuePtr src;
  ConstValuePtr dst;
};
struct InstructionFunctionCall : public Instruction {
 public:
  explicit InstructionFunctionCall(ConstValuePtr&& function_call);
  void accept(AbstractVisitorInst* v) const override;

  ConstValuePtr function_call;
};
struct InstructionWhileLoop : public Instruction {
 public:
  InstructionWhileLoop(ConstValuePtr&& cond, ConstInstrPtr&& body);
  void accept(AbstractVisitorInst* v) const override;

  ConstValuePtr cond;
  ConstInstrPtr body;
};
struct InstructionIfStatement : public Instruction {
 public:
  InstructionIfStatement() = default;
  InstructionIfStatement(ConstValuePtr&& cond, ConstInstrPtr&& true_scope);
  void accept(AbstractVisitorInst* v) const override;

  ConstValuePtr cond;
  ConstInstrPtr true_scope;
};
struct InstructionBreak : public Instruction {
 public:
  void accept(AbstractVisitorInst* v) const override;
};
struct InstructionContinue : public Instruction {
 public:
  void accept(AbstractVisitorInst* v) const override;
};
struct InstructionDecl : public Instruction {
 public:
  explicit InstructionDecl(std::vector<ValuePtr>&& vars);
  void accept(AbstractVisitorInst* v) const override;

  std::vector<ConstValuePtr> variables;
}; /*
 * Scope.
 */
struct Scope : public Instruction {
 public:
  void accept(AbstractVisitorInst* v) const override;
  std::vector<ConstInstrPtr> instructions;
};

struct StructDecl : TypedNode {
  std::string name;
  VarType::MemberNameToIndex member_name_to_index;
  std::vector<ConstVarTypePtr> member_types;
};

/*
 * Function.
 */
struct Function : TypedNode {
 public:
  std::string name;
  std::vector<ConstValuePtr> args;
  ConstInstrPtr scope;
  int64_t return_dim = 0;
};
}  // namespace ast

}  // namespace frontend
