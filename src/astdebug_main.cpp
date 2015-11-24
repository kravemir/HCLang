/*
 * HCLang - Highly Concurrent Language
 * Copyright (c) 2015 Miroslav Kravec
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <iostream>
#include <map>

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
#include "llvm/Pass.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include <cctype>
#include <cstdio>
#include <map>
#include <string>

#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>
#include <vector>

#include "parser.h"
#include "printer.h"

#include "base.hpp"

using namespace std;
using namespace llvm;

void register_malloc(llvm::Module *module) {
    std::vector<llvm::Type*> arg_types;
    arg_types.push_back(Type::getInt32Ty(getGlobalContext()));

    FunctionType* type = FunctionType::get(
            Type::getInt8PtrTy(getGlobalContext()), arg_types, false);

    Function *func = Function::Create(
                type, llvm::Function::ExternalLinkage,
                llvm::Twine("malloc"),
                module
           );
    func->setCallingConv(llvm::CallingConv::C);
}

void register_printf(llvm::Module *module) {
    std::vector<llvm::Type*> printf_arg_types;
    printf_arg_types.push_back(llvm::Type::getInt8PtrTy(getGlobalContext()));

    llvm::FunctionType* printf_type =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(getGlobalContext()), printf_arg_types, true);

    llvm::Function *func = llvm::Function::Create(
                printf_type, llvm::Function::ExternalLinkage,
                llvm::Twine("printf"),
                module
           );
    func->setCallingConv(llvm::CallingConv::C);
}

//static FunctionPassManager *TheFPM;
static ExecutionEngine *TheExecutionEngine;

extern "C" {
    Executor *executor;
}

int main(int argc, char **argv)
{
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    executor = mtexecutor_new();

    vector<Token> tokens = lexerFile(argv[1]);

    Parser parser(tokens, &std::cout);
    StatementList * list = parser.file();

    Printer p(std::cout);
    cout << "Statements " << list->size() << endl;
    for( Statement *s : *list )
        s->print(p);

    llvm::LLVMContext &llvmContext = llvm::getGlobalContext();
    SMDiagnostic error;
    //Module *m = parseIRFile("decl.ll", error, llvmContext);
    //std::unique_ptr<llvm::Module> Owner = llvm::make_unique<llvm::Module>("my cool jit", llvmContext);
    //std::unique_ptr<llvm::Module> Owner = std::unique_ptr<llvm::Module>(m);
    std::unique_ptr<llvm::Module> Owner = parseIRFile("decl.ll", error, llvmContext);
    Module *TheModule = Owner.get();
  /*TheModule->addModuleFlag(Module::Warning, "Debug Info Version",
                           DEBUG_METADATA_VERSION);*/
  /*if (Triple(sys::getProcessTriple()).isOSDarwin())
    TheModule->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);*/

    ContextStorage  s;
    Context         ctx(&s);

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

    TheExecutionEngine->addGlobalMapping(TheModule->getFunction("system_init"), (void*)system_init);

    //FunctionPassManager OurFPM(TheModule);

    // Set up the optimizer pipeline.  Start with registering info about how the
    // target lays out data structures.
    TheModule->setDataLayout(*TheExecutionEngine->getDataLayout());
#if 0
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
#endif

    // Set the global so the code gen can use this.
    //TheFPM = &OurFPM;

    s.module = TheModule;
    register_printf(TheModule);
    register_malloc(TheModule);

    for( Statement *s : *list )
        s->codegen(&ctx);


    //s.module->dump();
    
    TheExecutionEngine->finalizeObject();


    // JIT the function, returning a function pointer.
    Function *LF = TheModule->getFunction("main");
    void *FPtr = TheExecutionEngine->getPointerToFunction(LF);

    // Cast it to the right type (takes no arguments, returns a double) so we
    // can call it as a native function.
    void (*FP)() = (void (*)())(intptr_t)FPtr;
    FP();

    executor->run(executor);

    return 0;
}
