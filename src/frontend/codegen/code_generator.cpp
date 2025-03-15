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
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <cstdlib>
#include <map>
#include <string>
#include <system_error>
#include <vector>

#include "frontend/ast/ast.h"
#include "frontend/diagnostic/debug.h"
#include "frontend/visitor/IRInstructionGen.h"

namespace frontend {
CodeGenerator::CodeGenerator()
    : module_("my compiler!!!", context_), builder_(context_) {}
void CodeGenerator::generateCode(const Program& program,
                                 const std::string& output_filename) {
  /*
   * Generate target code
   */
  for (const auto& f : program.functions) {
    // generate function declaration

    auto allocatedVariables = functionSetup(f);
    IRInstructionGen irgen(builder_, context_, module_, allocatedVariables);
    generateLLVMIR(f, irgen);
  }

  // module_.print(llvm::errs(), nullptr);

  llvmVerifyGeneratedIr();

  llvmOptimPass();

  llvmCodegenPass(output_filename + ".asm",
                  llvm::CodeGenFileType::CGFT_AssemblyFile);

  llvmCodegenPass(output_filename, llvm::CodeGenFileType::CGFT_ObjectFile);
  // module_.print(llvm::errs(), nullptr);
}

void CodeGenerator::llvmVerifyGeneratedIr() const {
  DEBUG_PRINT("==========================================\n");
  DEBUG_PRINT("Verifying correctness of generated LLVM IR\n");
  bool isError = llvm::verifyModule(module_, &llvm::errs());
  (void)isError;
  assert(!isError);
}

void CodeGenerator::llvmOptimPass() {
  DEBUG_PRINT("==========================================\n");
  DEBUG_PRINT("running opt passes ............\n");
  // llvm::FunctionPassManager fpm;
  llvm::LoopAnalysisManager loopAnalysisManager;
  llvm::FunctionAnalysisManager functionAnalysisManager;
  llvm::CGSCCAnalysisManager cgsccAnalysisManager;
  llvm::ModuleAnalysisManager moduleAnalysisManager;

  llvm::PassBuilder pb;

  // Register all the basic analyses with the managers.
  pb.registerModuleAnalyses(moduleAnalysisManager);
  pb.registerCGSCCAnalyses(cgsccAnalysisManager);
  pb.registerFunctionAnalyses(functionAnalysisManager);
  pb.registerLoopAnalyses(loopAnalysisManager);
  pb.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager,
                          cgsccAnalysisManager, moduleAnalysisManager);

  // Create the pass manager.
  // This one corresponds to a typical -O2 optimization pipeline.
  llvm::ModulePassManager optimizePassManager =
      pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
  optimizePassManager.run(module_, moduleAnalysisManager);
}

void CodeGenerator::llvmCodegenPass(const std::string& filename,
                                    llvm::CodeGenFileType file_type) {
  //  Initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
  auto targetTriple = llvm::sys::getDefaultTargetTriple();
  DEBUG_PRINT("target triple: " << target_triple << "\n");
  // target_triple = "x86_64-pc-linux-gnu";

  const auto* cpu = "generic";
  const auto* features = "";
  std::string errorStr;

  const auto* target =
      llvm::TargetRegistry::lookupTarget(targetTriple, errorStr);
  if (!target) {
    FRONTEND_ERROR(errorStr);
  }

  llvm::TargetOptions opt;
  llvm::TargetMachine* targetMachine = target->createTargetMachine(
      targetTriple, cpu, features, opt, llvm::Reloc::PIC_);

  module_.setDataLayout(targetMachine->createDataLayout());
  module_.setTargetTriple(targetTriple);

  std::error_code errorCode;
  llvm::raw_fd_ostream dest(filename, errorCode, llvm::sys::fs::OF_None);

  if (errorCode) {
    FRONTEND_ERROR("Could not open file: " + errorCode.message());
  }

  llvm::legacy::PassManager pass;

  if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
    FRONTEND_ERROR("target_machine can't emit a file of this type\n");
    exit(1);
  }

  pass.run(module_);
  dest.flush();
}

void CodeGenerator::generateLLVMIR(const ast::FunctionPtr& f,
                                   IRInstructionGen& irgen) {
  irgen.get(*f->scope.get());
}

std::map<const ast::Variable*, llvm::Value*> CodeGenerator::functionSetup(
    const ast::FunctionPtr& f) {
  std::vector<llvm::Type*> argLlvmTypes(f->args.size());
  for (int i = 0; i < f->args.size(); i++) {
    argLlvmTypes[i] = f->args[i]->type->getLlvmInRegType(context_);
  }
  if (f->type->isArray() || f->type->isStruct()) {
    // last parameter is the address of the returned object
    argLlvmTypes.push_back(f->type->getLlvmInRegType(context_));
  }

  llvm::Type* llvmRetType = f->type->getLlvmInRegType(context_);
  llvm::FunctionType* functionType =
      llvm::FunctionType::get(llvmRetType, argLlvmTypes, false);
  llvm::Function* llvmFunc = llvm::Function::Create(
      functionType, llvm::Function::ExternalLinkage, f->name, module_);

  // entry block
  llvm::BasicBlock* entryBlock =
      llvm::BasicBlock::Create(context_, "entry", llvmFunc);
  builder_.SetInsertPoint(entryBlock);

  unsigned int i = 0;
  std::map<const ast::Variable*, llvm::Value*> allocatedVariables;
  for (const auto& var : f->args) {
    // Note: intentionally skips last llvm func arg if it is a return value arg
    auto* llvmArg = llvmFunc->getArg(i);
    setupFunctionArgs(allocatedVariables, llvmArg, var);
    i++;
  }
  return allocatedVariables;
}
void CodeGenerator::setupFunctionArgs(
    std::map<const ast::Variable*, llvm::Value*>& allocated_variables,
    llvm::Argument* llvm_arg, const ast::ConstValuePtr& var) {
  const auto* arg = dynamic_cast<const ast::Variable*>(var.get());
  if (!arg) {
    FRONTEND_ERROR("error: arg in function definition is not a variable\n");
  }
  const auto& currArg = var;

  if (currArg->type->isObject()) {
    // allocate stack space for pass-by-value param
    auto* stackPtr = this->builder_.CreateAlloca(
        arg->type->getLlvmStackAllocTy(this->context_), nullptr,
        "pass-by-copy");
    this->builder_.CreateMemCpy(stackPtr, llvm::MaybeAlign(), llvm_arg,
                                llvm::MaybeAlign(),
                                currArg->type->getObjectSize());
    allocated_variables[arg] = stackPtr;
  } else if (currArg->type->isRef()) {
    allocated_variables[arg] = llvm_arg;
  } else {
    allocated_variables[arg] =
        this->builder_.CreateAlloca(arg->type->getLlvmInRegType(this->context_),
                                    nullptr, "pass-by-copy-atomic");
    this->builder_.CreateStore(llvm_arg, allocated_variables[arg]);
  }
}
}  // namespace frontend
