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
#ifndef HCLANG_AST_STMT_FOR_H
#define HCLANG_AST_STMT_FOR_H

#include "base.h"

class ForStmt : public Statement {
public:
    ForStmt(std::string target_name, MValueAST *inval, StatementList *stmts):
            target_name(target_name),
            inval(inval),
            stmts(stmts)
    {}

    virtual void codegen(Context *ctx);
    virtual void collectAlloc ( Context* ctx );

    virtual void print(Printer &p) const;
private:
    std::string target_name;
    MValueAST *inval;
    StatementList *stmts;

    llvm::AllocaInst* iPtr = 0;
};

#endif //HCLANG_AST_STMT_FOR_H
