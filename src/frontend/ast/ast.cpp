#include "frontend/ast/ast.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "frontend/diagnostic/debug.h"
#include "frontend/visitor/AbstractVisitorInst.h"
#include "frontend/visitor/AbstractVisitorValue.h"

namespace frontend::ast {
BinOpId stringToBinop(const std::string& op) {
  if (op == "+")
    return ADD;
  if (op == "*")
    return MUL;
  if (op == "&")
    return AND;
  if (op == "-")
    return SUB;
  if (op == "<<")
    return SHL;
  if (op == ">>")
    return SHR;
  if (op == "<")
    return LT;
  if (op == ">")
    return GT;
  if (op == "<=")
    return LEQ;
  if (op == ">=")
    return GEQ;
  if (op == "==")
    return EQ;

  FRONTEND_ERROR("Unknown binop: " + op);
}
std::string binopToString(BinOpId op) {
  switch (op) {
    case ADD:
      return "+";
    case MUL:
      return "*";
    case AND:
      return "&";
    case SUB:
      return "-";
    case SHL:
      return "<<";
    case SHR:
      return ">>";
    case LT:
      return "<";
    case GT:
      return ">";
    case LEQ:
      return "<=";
    case GEQ:
      return ">=";
    case EQ:
      return "=";
    default:
      return "none";
  }
}
void Scope::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

Value::~Value() = default;

// anonymous namespace
namespace {
// https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const/8147213#8147213
// 20.7.2.2.6 shared_ptr creation [util.smartptr.shared.get], paragraph 1
class DerivedVariable : public Variable {
 public:
  DerivedVariable() = default;
  ~DerivedVariable() override = default;
};
}  // namespace

std::map<const Function*, std::map<std::string, std::shared_ptr<Variable>>>
    Variable::variables_;
std::shared_ptr<Variable> Variable::get(const std::string& name,
                                        const Function* f) {
  if (variables_.find(f) == variables_.end()) {
    variables_[f] = std::map<std::string, std::shared_ptr<Variable>>();
  }

  if (variables_[f].find(name) == variables_[f].end()) {
    auto newVar = std::make_shared<DerivedVariable>();
    newVar->name = name;
    variables_[f][name] = std::move(newVar);
  }
  return variables_[f][name];
}

Integer::Integer(int64_t value)
    : value(value), Value(VarType::getLiteralType("int64")) {}

// ========================
// define all the accept functions from ast.h here
// ========================

// ========== Items ==========
void Variable::accept(AbstractVisitorValue* v) const {
  v->visit(this);
}

void Integer::accept(AbstractVisitorValue* v) const {
  v->visit(this);
}

FunctionName::FunctionName(std::string&& name, ConstVarTypePtr ret)
    : name(std::move(name)), return_type(std::move(ret)) {}

void FunctionName::accept(AbstractVisitorValue* v) const {
  v->visit(this);
}

FunctionCall::FunctionCall(ConstValuePtr&& function,
                           std::vector<ConstValuePtr>&& args)
    : function(std::move(function)), args(std::move(args)) {}

void FunctionCall::accept(AbstractVisitorValue* v) const {
  v->visit(this);
}

BinaryOperation::BinaryOperation(BinOpId op, ConstValuePtr&& lhs,
                                 ConstValuePtr&& rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

void BinaryOperation::accept(AbstractVisitorValue* v) const {
  v->visit(this);
}

ArrayAccess::ArrayAccess(ConstValuePtr&& var,
                         std::vector<ConstValuePtr>&& indices, uint64_t)
    : var(std::move(var)), indices(std::move(indices)) {}

void ArrayAccess::accept(AbstractVisitorValue* v) const {
  v->visit(this);
}

ArrayAllocate::ArrayAllocate(ConstValuePtr&& length, ConstValuePtr&& elem_value)
    : length(std::move(length)), elem_value(std::move(elem_value)) {}

void ArrayAllocate::accept(AbstractVisitorValue* v) const {
  v->visit(this);
}

InstructionReturn::InstructionReturn(ConstValuePtr&& val)
    : val(std::move(val)) {}

InstructionReturn::InstructionReturn() : val(nullptr) {}

// ========== Instructions ==========
void InstructionReturn::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

InstructionAssignment::InstructionAssignment(ConstValuePtr&& dst,
                                             ConstValuePtr&& src)
    : dst(std::move(dst)), src(std::move(src)) {}

void InstructionAssignment::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

InstructionFunctionCall::InstructionFunctionCall(ConstValuePtr&& function_call)
    : function_call(function_call) {}

void InstructionFunctionCall::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

InstructionWhileLoop::InstructionWhileLoop(ConstValuePtr&& cond,
                                           ConstInstrPtr&& body)
    : cond(std::move(cond)), body(std::move(body)) {}

void InstructionWhileLoop::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

InstructionIfStatement::InstructionIfStatement(ConstValuePtr&& cond,
                                               ConstInstrPtr&& true_scope)
    : cond(std::move(cond)), true_scope(std::move(true_scope)) {}

void InstructionIfStatement::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

void InstructionBreak::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

void InstructionContinue::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

InstructionDecl::InstructionDecl(std::vector<ValuePtr>&& vars) {
  for (auto& var : vars) {
    this->variables.push_back(std::move(var));
  }
  vars.clear();
}

void InstructionDecl::accept(AbstractVisitorInst* v) const {
  v->visit(this);
}

}  // namespace frontend::ast
