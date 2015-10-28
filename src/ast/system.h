#ifndef HCLANG_AST_SYSTEM_H
#define HCLANG_AST_SYSTEM_H

#include "base.h"

struct SystemType : MValueType {
    int slotCount = 0;
    std::map<std::string,int> slotIds;
    std::vector<llvm::Function*> slots;
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
    virtual void print(Printer &p) const;

private:
    llvm::Function* codegen_msghandler(Context *ctx);

    std::string name;
    StatementList *stmts;
};

#endif // HCLANG_AST_SYSTEM_H
