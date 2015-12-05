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
#ifndef HCLANG_AST_SYSTEM_H
#define HCLANG_AST_SYSTEM_H

#include "base.h"

class SlotType;

struct SystemType : MValueType {
    int slotCount = 0;
    std::map<std::string,int> slotIds;
    std::vector<llvm::Function*> slots;
    std::vector<SlotType*> slotTypes;
    llvm::Function *fn_new;

    std::vector<std::pair<std::string,MValueType*>> variables;


    virtual llvm::Type* llvmType() const {
        return _llvmType;
    }
    virtual MValue* getChild(MValue *src, std::string name);

    llvm::Type* _llvmType;
};

struct SystemDecl : Statement {
    SystemDecl( std::string name, StatementList *list ):
        name(name),
        stmts(list)
    {}

    virtual void codegen(Context *ctx);
    virtual void collectAlloc ( Context* ctx ) {};

    virtual void print(Printer &p) const;

private:
    llvm::Function* codegen_msghandler(Context *ctx);

    std::string name;
    StatementList *stmts;
};

#endif // HCLANG_AST_SYSTEM_H
