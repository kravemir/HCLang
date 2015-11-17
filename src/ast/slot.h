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

#include "base.h"
#include "tuple.h"

class SlotType : public MValueType {
public:
    static SlotType* create ( Context *ctx, TupleType *argsType );

private:
    SlotType ( llvm::Type* _llvmType, TupleType *argsTupleType ) :
        MValueType ( _llvmType,false,0 ),
        argsTupleType ( argsTupleType )
    {}

public:
    TupleType* const argsTupleType;
};

class SlotTypeAST : public MTypeAST {
public:
    SlotTypeAST ( MTupleTypeAST *args ) :
        args ( args )
    {}

    virtual MValueType* codegen ( Context* ctx );

public:
    MTupleTypeAST* const args;
};

class SlotDecl : public Statement {
public:
    SlotDecl ( std::string name, MTupleTypeAST *args, StatementList *list ) :
        name ( name ),
        args ( args ),
        stmts ( list )
    {}

    virtual void codegen ( Context *ctx );

    virtual void print ( Printer &p ) const;
    virtual void collectSystemDecl ( Context *ctx ) const;
private:
    std::string name;
    MTupleTypeAST *args;
    StatementList *stmts;
};

#endif // HCLANG_AST_SLOT_H
