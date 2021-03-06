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
#ifndef HCLANG_AST_SLOT_H
#define HCLANG_AST_SLOT_H

#include "ast/base.h"

#include "ast/types/tuple.h"

class SlotType : public MValueType {
public:
    static SlotType* create ( Context *ctx, TupleType *argsType, MValueType *returnType );

private:
    SlotType ( llvm::Type* _llvmType, TupleType *argsTupleType, MValueType *returnType ) :
        MValueType ( _llvmType,false,0 ),
        argsTupleType ( argsTupleType ),
        returnType(returnType)
    {}


public:
    virtual void codegenSendTo(MValue *value, MValue *msg) override;

public:
    TupleType* const argsTupleType;
    MValueType* const returnType;
    llvm::Function *putMsgFn; // TODO
};

class SlotTypeAST : public MTypeAST {
public:
    SlotTypeAST ( MTupleTypeAST *args, MTypeAST *returnType ) :
        args ( args ),
        returnType(returnType)
    {}

    virtual MValueType* codegen ( Context* ctx );

public:
    MTupleTypeAST* const args;
    MTypeAST* const returnType;
};

class SlotDecl : public Statement {
public:
    SlotDecl ( std::string name, MTupleTypeAST *args, MTypeAST *returnType, StatementList *list ) :
        name ( name ),
        args ( args ),
        returnType (returnType),
        stmts ( list )
    {}

    virtual void codegen ( Context *ctx );

    virtual void print ( Printer &p ) const;
    virtual void collectSystemDecl ( Context *ctx ) const;
    virtual void collectAlloc ( Context* ctx ) {};

private:
    std::string name;
    MTupleTypeAST *args;
    MTypeAST *returnType;
    StatementList *stmts;
    mutable SlotType *type;
};

#endif // HCLANG_AST_SLOT_H
