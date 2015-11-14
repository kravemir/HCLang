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
 */
#ifndef HCLANG_AST_PROCEDURE_H
#define HCLANG_AST_PROCEDURE_H

#include "base.h"

struct ProcedureType : MValueType {
    ProcedureType(llvm::Type* _llvmType, MValueType *retType):
        MValueType(_llvmType,true,retType)
    {}
};

struct ProcedureAsyncInstanceType : MValueType {
    ProcedureAsyncInstanceType(llvm::Type* _llvmType):
        MValueType(_llvmType)
    {}
};

struct ProcedureAsyncType : MValueType {
    ProcedureAsyncType(llvm::Type* _llvmType, MValueType *retType):
        MValueType(_llvmType,true,retType)
    {}
};

class ProcedureDecl : public Statement {
public:
    ProcedureDecl( std::string name, MTupleTypeAST *args, MTypeAST *retType, StatementList *list, bool async ):
        name(name),
        args(args),
        returnType(retType),
        stmts(list),
        async(async)
    {
        assert(returnType);
    }

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;

private:
    std::string name;
    MTupleTypeAST *args;
    MTypeAST *returnType;
    StatementList *stmts;
    bool async;
};

#endif // HCLANG_AST_PROCEDURE_H
