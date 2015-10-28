#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
//#include "llvm/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
using namespace llvm;


static std::string IdentifierStr; // Filled in if tok_identifier

static Module *TheModule;
static IRBuilder<> Builder(getGlobalContext());
static std::map<std::string, Value *> NamedValues;
static FunctionPassManager *TheFPM;
static ExecutionEngine *TheExecutionEngine;


//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

/// putchard - putchar that takes a double and returns 0.
extern "C" double putchard(double X) {
  putchar((char)X);
  return 0;
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
void register_printf() {
    // Make the function type:  double(double,double) etc.
    std::vector<Type *> ArgsTypes;
    ArgsTypes.push_back(Type::getInt8PtrTy(getGlobalContext()));
    ArgsTypes.push_back(Type::getInt32Ty(getGlobalContext()));

    FunctionType *FT =
      FunctionType::get(Type::getInt32Ty(getGlobalContext()), ArgsTypes, false);

    Function::Create(FT, Function::ExternalLinkage, "printf", TheModule);
}

#include "base.hpp"
#include "csamples/hellosystem.h"

int main() {
#if 0
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  LLVMContext &Context = getGlobalContext();

  // Prime the first token.
  //fprintf(stderr, "ready> ");
  //getNextToken();

  // Make the module, which holds all the code.
  std::unique_ptr<Module> Owner = make_unique<Module>("my cool jit", Context);
  TheModule = Owner.get();

  // Create the JIT.  This takes ownership of the module.
  std::string ErrStr;
  TheExecutionEngine =
      EngineBuilder(std::move(Owner))
          .setErrorStr(&ErrStr)
          .setMCJITMemoryManager(llvm::make_unique<SectionMemoryManager>())
          .create();
  if (!TheExecutionEngine) {
    fprintf(stderr, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
    exit(1);
  }

  FunctionPassManager OurFPM(TheModule);

  // Set up the optimizer pipeline.  Start with registering info about how the
  // target lays out data structures.
  TheModule->setDataLayout(TheExecutionEngine->getDataLayout());
  OurFPM.add(new DataLayoutPass());
  // Provide basic AliasAnalysis support for GVN.
  OurFPM.add(createBasicAliasAnalysisPass());
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  OurFPM.add(createInstructionCombiningPass());
  // Reassociate expressions.
  OurFPM.add(createReassociatePass());
  // Eliminate Common SubExpressions.
  OurFPM.add(createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  OurFPM.add(createCFGSimplificationPass());

  OurFPM.doInitialization();

  // Set the global so the code gen can use this.
  TheFPM = &OurFPM;

  // Run the main "interpreter loop" now.
  //MainLoop();
  register_printf();

  TheFPM = 0;


  Executor *e = executor_new();
  HelloSystem *hs = hellosystem_new(e);
  system_putMsg( (System*)hs, 0, 0 );
  system_putMsg( (System*)hs, 1, 0 );

  executor_mainloop( e );

  // Print out all of the generated code.
  TheModule->dump();

  return 0;
#endif
}
