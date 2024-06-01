#include "frontend/visitor/DumpAST.h"
namespace frontend {

void DumpAST::dump_program(const Program& program) {
  stream_ << "dump!\n";
  for (const auto& function : program.functions) {
    prefix_ = "\n";
    dump_function(*function);
  }
  stream_ << std::endl;
}

void DumpAST::dump_function(const ast::Function& function) {
  prefix_ = "\n";
  stream_ << prefix_ << "define " << function.type->get_type_name() << " "
          << function.name;
  std::string saved_prefix = prefix_;
  prefix_ = " ";
  for (const auto& arg : function.args) {
    dump_value(*arg);
    stream_ << " ";
  }
  prefix_ = saved_prefix;
  prefix_ += " |";
  dump_instruction(*function.scope);
  prefix_ = saved_prefix;
}

void DumpAST::dump_instruction(const ast::Instruction& instruction) {
  stream_ << prefix_;
  instruction.accept(this);
}

void DumpAST::dump_value(const ast::Value& value) {
  stream_ << prefix_;
  value.accept(this);
}

void DumpAST::visit(const ast::Variable* var) {
  stream_ << var->type->get_type_name() << " " << var->name;
}
void DumpAST::visit(const ast::Integer* num) {
  stream_ << num->value << " " << num->type->get_type_name();
}
void DumpAST::visit(const ast::FunctionName* func_name) {
  stream_ << func_name->return_type->get_type_name() << " " << func_name->name;
}
void DumpAST::visit(const ast::BinaryOperation* bin_op) {
  stream_ << ast::binop_to_string(bin_op->op)
          << " type: " << bin_op->type->get_type_name();
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  dump_value(*bin_op->lhs);
  dump_value(*bin_op->rhs);
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::FunctionCall* call) {

  stream_ << "func call ";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  dump_value(*call->function);
  for (const auto& arg : call->args) {
    dump_value(*arg);
  }
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::ArrayAccess* access) {
  stream_ << "array access" << " type: " << access->type->get_type_name();
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  dump_value(*access->var);
  dump_value(*access->indices.back());
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::ArrayAllocate* alloc) {
  stream_ << "array allocate" << " type: " << alloc->type->get_type_name();
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  dump_value(*alloc->length);
  dump_value(*alloc->elem_value);
  prefix_ = saved_prefix;
}

// ========== Instructions ==========
void DumpAST::visit(const ast::InstructionReturn* ret) {
  stream_ << "return ";
  if (ret->val != nullptr) {
    std::string saved_prefix = prefix_;
    prefix_ += " |";
    dump_value(*ret->val);
    prefix_ = saved_prefix;
  }
}
void DumpAST::visit(const ast::InstructionAssignment* assign) {
  stream_ << "assign";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  dump_value(*assign->dst);
  dump_value(*assign->src);
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::InstructionFunctionCall* call) {
  stream_ << "call";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  dump_value(*call->function_call);
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::InstructionWhileLoop* loop) {
  stream_ << "while stmt";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  dump_value(*loop->cond);
  dump_instruction(*loop->body);
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::InstructionIfStatement* if_stmt) {
  stream_ << "if stmt";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  dump_value(*if_stmt->cond);
  dump_instruction(*if_stmt->true_scope);
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::InstructionBreak* brk) {
  stream_ << "BREAK";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::InstructionContinue* cont) {
  stream_ << "CONTINUe";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  prefix_ = saved_prefix;
}
void DumpAST::visit(const ast::InstructionDecl* decl) {
  stream_ << "decl";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  for (const auto& var : decl->variables) {
    dump_value(*var);
  }
  prefix_ = saved_prefix;
}

void DumpAST::visit(const ast::Scope* scope) {
  stream_ << "scope";
  std::string saved_prefix = prefix_;
  prefix_ += " |";
  for (const auto& inst : scope->instructions) {
    dump_instruction(*inst);
  }
  prefix_ = saved_prefix;
}
DumpAST::DumpAST() : stream_(std::cout) {}

}  // namespace frontend
