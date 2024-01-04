#include "frontend/code_generator.h"

#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

#include <cstdlib>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <system_error>
#include <vector>

#include "frontend/ast.h"
#include "frontend/visitor/IRInstructionGen.h"

namespace frontend {
CodeGenerator::CodeGenerator()
    : context_(), module_("my compiler!!!", context_), builder_(context_) {}
void CodeGenerator::generate_code(const Program& program,
                                  const std::string& output_filename) {
  /*
   * Generate target code
   */
  for (const auto& f : program.functions) {
    // generate function declaration

    auto allocated_variables = function_setup(f);
    IRInstructionGen irgen(builder_, context_, module_, allocated_variables);
    generate_llvm_ir(f, irgen);
  }

  module_.print(llvm::errs(), nullptr);

  llvm_verify_generated_ir();

  llvm_optim_pass();

  llvm_codegen_pass(output_filename + ".asm",
                    llvm::CodeGenFileType::AssemblyFile);

  llvm_codegen_pass(output_filename, llvm::CodeGenFileType::ObjectFile);
  module_.print(llvm::errs(), nullptr);
}

void CodeGenerator::llvm_verify_generated_ir() const {
  std::cout << "==========================================\n";
  std::cout << "Verifying correctness of generated LLVM IR\n";
  bool is_error = llvm::verifyModule(module_, &llvm::errs());
  if (is_error) {
    std::cout << "error: generated LLVM IR is not correct" << std::endl;
    exit(1);
  } else {
    std::cout << "passed!\n";
  }
}

void CodeGenerator::llvm_optim_pass() {
  std::cout << "==========================================\n";
  std::cout << "running opt passes ............\n";
  // llvm::FunctionPassManager fpm;
  llvm::LoopAnalysisManager loop_analysis_manager;
  llvm::FunctionAnalysisManager function_analysis_manager;
  llvm::CGSCCAnalysisManager cgscc_analysis_manager;
  llvm::ModuleAnalysisManager module_analysis_manager;

  llvm::PassBuilder pb;

  // Register all the basic analyses with the managers.
  pb.registerModuleAnalyses(module_analysis_manager);
  pb.registerCGSCCAnalyses(cgscc_analysis_manager);
  pb.registerFunctionAnalyses(function_analysis_manager);
  pb.registerLoopAnalyses(loop_analysis_manager);
  pb.crossRegisterProxies(loop_analysis_manager, function_analysis_manager,
                          cgscc_analysis_manager, module_analysis_manager);

  // Create the pass manager.
  // This one corresponds to a typical -O2 optimization pipeline.
  llvm::ModulePassManager optimize_pass_manager =
      pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
  optimize_pass_manager.run(module_, module_analysis_manager);
}

void CodeGenerator::llvm_codegen_pass(const std::string& filename,
                                      llvm::CodeGenFileType file_type) {
  //  Initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
  auto target_triple = llvm::sys::getDefaultTargetTriple();
  std::cout << "target triple: " << target_triple << std::endl;
  // target_triple = "x86_64-pc-linux-gnu";

  auto cpu = "generic";
  auto features = "";
  std::string error_str;

  auto target = llvm::TargetRegistry::lookupTarget(target_triple, error_str);
  if (!target) {
    std::cout << error_str << std::endl;
    exit(1);
  }

  llvm::TargetOptions opt;
  llvm::TargetMachine* target_machine = target->createTargetMachine(
      target_triple, cpu, features, opt, llvm::Reloc::PIC_);

  module_.setDataLayout(target_machine->createDataLayout());
  module_.setTargetTriple(target_triple);

  std::error_code error_code;
  llvm::raw_fd_ostream dest(filename, error_code, llvm::sys::fs::OF_None);

  if (error_code) {
    llvm::errs() << "Could not open file: " << error_code.message();
    exit(1);
  }

  llvm::legacy::PassManager pass;

  if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
    llvm::errs() << "target_machine can't emit a file of this type\n";
    exit(1);
  }

  pass.run(module_);
  dest.flush();

  llvm::outs() << "Wrote " << filename << "\n";
}

void CodeGenerator::generate_llvm_ir(const ast::FunctionPtr& f,
                                     IRInstructionGen& irgen) {
  irgen.get(*f->scope.get());
}

std::map<const ast::Variable*, llvm::Value*> CodeGenerator::function_setup(
    const ast::FunctionPtr& f) {
  std::vector<llvm::Type*> arg_llvm_types(f->args.size());
  for (int i = 0; i < f->args.size(); i++) {
    arg_llvm_types[i] = f->args[i]->type->get_llvm_in_reg_type(context_);
  }
  if (f->type->is_array() || f->type->is_struct()) {
    // last parameter is the address of the returned object
    arg_llvm_types.push_back(f->type->get_llvm_in_reg_type(context_));
  }

  llvm::Type* llvm_ret_type = f->type->get_llvm_in_reg_type(context_);
  llvm::FunctionType* function_type =
      llvm::FunctionType::get(llvm_ret_type, arg_llvm_types, false);
  llvm::Function* llvm_func = llvm::Function::Create(
      function_type, llvm::Function::ExternalLinkage, f->name, module_);

  // entry block
  llvm::BasicBlock* entry_block =
      llvm::BasicBlock::Create(context_, "entry", llvm_func);
  builder_.SetInsertPoint(entry_block);

  unsigned int i = 0;
  std::map<const ast::Variable*, llvm::Value*> allocated_variables;
  for (const auto& var : f->args) {
    // Note: intentionally skips last llvm func arg if it is a return value arg
    auto llvm_arg = llvm_func->getArg(i);
    setup_function_args(allocated_variables, llvm_arg, var);
    i++;
  }
  return allocated_variables;
}
void CodeGenerator::setup_function_args(
    std::map<const ast::Variable*, llvm::Value*>& allocated_variables,
    llvm::Argument* llvm_arg, const ast::ConstValuePtr& var) {
  const auto* arg = dynamic_cast<const ast::Variable*>(var.get());
  if (!arg) {
    std::cout << "error: arg in function definition is not a variable"
              << std::endl;
    exit(1);
  }
  const auto& curr_arg = var;

  if (curr_arg->type->is_object()) {
    // allocate stack space for pass-by-value param
    auto* stack_ptr = this->builder_.CreateAlloca(
        arg->type->get_llvm_stack_alloc_ty(this->context_), nullptr,
        "pass-by-copy");
    this->builder_.CreateMemCpy(stack_ptr, llvm::MaybeAlign(), llvm_arg,
                                llvm::MaybeAlign(),
                                curr_arg->type->get_object_size());
    allocated_variables[arg] = stack_ptr;
  } else if (curr_arg->type->is_ref()) {
    allocated_variables[arg] = llvm_arg;
  } else {
    allocated_variables[arg] = this->builder_.CreateAlloca(
        arg->type->get_llvm_in_reg_type(this->context_), nullptr,
        "pass-by-copy-atomic");
    this->builder_.CreateStore(llvm_arg, allocated_variables[arg]);
  }
}
}  // namespace frontend
