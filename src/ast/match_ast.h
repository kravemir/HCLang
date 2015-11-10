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
#ifndef HCLANG_MATCH_AST_H
#define	HCLANG_MATCH_AST_H

#include "base.h"

struct MMatchExpr {
    virtual ~MMatchExpr();
    
    virtual MValue* codegenMatchCond(Context &ctx, MValue *value);
};

struct MMatchVar : MMatchExpr {
    std::string name;
    MTypeAST *type;
    
    virtual MValue* codegenMatchCond(Context &ctx, MValue *value);
};

struct MMatchVarObject : MMatchExpr {
    std::string name;
    
    virtual MValue* codegenMatchCond(Context &ctx, MValue *value);
};

struct MMatchTuple : MMatchExpr {
    std::vector<MMatchExpr*> expr;
    
    
    virtual MValue* codegenMatchCond(Context &ctx, MValue *value);
};

#endif	/* HCLANG_MATCH_AST_H */

