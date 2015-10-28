/* 
 * File:   match_ast.h
 * Author: miroslav
 *
 * Created on October 21, 2015, 8:59 PM
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

