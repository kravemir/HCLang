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
#ifndef HCLANG_AST_BASE_H
#define HCLANG_AST_BASE_H

#include <set>
#include <map>
#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

#include "lexer.h"

extern llvm::IRBuilder<> Builder;
extern llvm::LLVMContext &lctx;

class Printer;

struct MValue;
struct Context;
struct SystemType;

struct MValueType {
    llvm::Type* const _llvmType;
    const bool callable;
    MValueType * const _callReturnType;

    MValueType( llvm::Type* _llvmType = 0, bool callable = false, MValueType *callRet = 0 ):
        _llvmType(_llvmType),
        callable(callable),
        _callReturnType(callRet)
    {
        // TODO assert(_llvmType)
        assert(!callable || _callReturnType);
    }

    virtual llvm::Type* llvmType() const { assert(_llvmType); return _llvmType; }

    virtual ~MValueType() {};

    virtual MValue* getChild(MValue *src, std::string name) { return 0; }
    virtual MValue* getArrayChild(MValue *src, llvm::Value *index) { return 0; }

    virtual std::pair<llvm::Value*,MValue*> matchCond(std::string targetName, MValue* src, Context *ctx) { return {0,0}; }

    virtual MValue* createConstructor(Context *ctx) { return 0; }

    virtual MValue* createCast(Context *ctx, MValue *src) { return 0; }
    virtual MValueType *callReturnType() { 
        assert(callable);
        return _callReturnType;
    };
};

struct TupleAST;
struct MTupleTypeAST;


struct IntType : MValueType {
    IntType(llvm::Type* type) : MValueType(type) { assert(type); };
};
struct StringType : MValueType {
    StringType(llvm::Type* type) : MValueType(type) { assert(type); };
};
struct VoidType : MValueType {
    VoidType(llvm::Type* type) : MValueType(type) { assert(type); };
};

struct MFunctionType : MValueType {
    MFunctionType(MValueType *retType) : MValueType(0,true,retType) {}

    MValueType *retType;
    virtual llvm::Type* llvmType() const {
        return retType->llvmType();
    }
};

struct MValue {
    MValueType *type;
    llvm::Value *_value;
    bool variable = false;
    llvm::Value *ptr = 0;

    MValue(MValueType *type, llvm::Value *_value, bool variable = false):
        type(type),
        _value(_value),
        variable(variable)
    {
        assert(type != 0);
        assert(_value != 0);
    }

    MValue* getChild(std::string name) {
        return type->getChild(this,name);
    }

    llvm::Value* value();
    void store(MValue *value);
};

struct ContextStorage {
    std::map<std::string,MValue*> values;
    std::map<std::string,MValue*> variables;
    std::map<std::string,MValueType*>   types;
    SystemType *system;
    llvm::Module *module;
    std::string prefix;
};

struct Context {
    ContextStorage *storage;

    std::map<std::string,MValue*> shadowed_values;
    std::set<std::string> extra_values;

    std::map<std::string,MValue*> shadowed_variables;
    std::set<std::string> extra_variables;

    void bindValue(std::string name, MValue* value) {
        MValue *old = getValue(name,false);
        if(old)
            shadowed_values[name] = old;
        storage->values[name] = value;
    }
    void addVariable(std::string name, MValue* value) {
        MValue *old = getVariable(name);
        if(old)
            shadowed_variables[name] = old;
        storage->variables[name] = value;
    }

    MValue* getVariable(std::string name){
        auto it = storage->values.find(name);
        if( it != storage->values.end() )
            return it->second;
        return 0;
    }
    MValue* getValue(std::string name, bool fallToVariable = true) {
        auto it = storage->values.find(name);
        if( it != storage->values.end() )
            return it->second;
        return 0;
    }

    Context(ContextStorage *storage):
        storage(storage)
    {}

    Context(Context *parent):
        storage(parent->storage)
    {}

    virtual ~Context() {
        // TODO
    };
};

struct SystemContext : Context {
    SystemType *shadowed_system;
    std::string oldprefix;


    SystemContext(Context *parent, std::string name, SystemType *type): Context(parent) {
        shadowed_system = storage->system;
        storage->system = type;
        oldprefix = storage->prefix;
        storage->prefix += "s" + name + "_";
    }
    ~SystemContext() {
        storage->system = shadowed_system;
        storage->prefix = oldprefix;
    }
};

struct Path : std::vector<std::string> {
};

class MTypeAST {
public:
    virtual MValueType* codegen(Context *ctx) = 0;
};

class MVoidTypeAST: public MTypeAST {
public:
    virtual MValueType* codegen ( Context* ctx ) {
        return new VoidType(llvm::Type::getVoidTy(llvm::getGlobalContext()));
    }
};

class MNameTypeAST : public MTypeAST {
public:
    MNameTypeAST(std::string name):
        name(name)
    {}

    virtual MValueType* codegen(Context *ctx) { 
        if( name == "int" )
            return new IntType(llvm::Type::getInt64Ty(ctx->storage->module->getContext()));
        else if( name == "String" )
            return new StringType(llvm::Type::getInt8PtrTy(ctx->storage->module->getContext()));
        return ctx->storage->types[name]; 
    };

private:
    std::string name;
};

class MValueAST {
public:
    virtual ~MValueAST() {};

    virtual MValue* codegen(Context *ctx, MValueType *type = 0) = 0;

    virtual std::string toString() const = 0;
};

class VarExpr : public MValueAST {
public:
    VarExpr(std::string str);

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    virtual std::string toString() const;

private:
    std::string str;
};

class GetChildAST : public MValueAST {
public:
    GetChildAST(MValueAST *val, std::string name):
        val(val),
        name(name)
    {}

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    virtual std::string toString() const { return val->toString() + "." + name; }

private:
    MValueAST *val;
    std::string name;
};

class SpawnExpr : public MValueAST {
public:
    SpawnExpr(std::string str, TupleAST *spawnArgs);

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    virtual std::string toString() const;

private:
    std::string name;
    TupleAST *spawnArgs;
};

class IntegerAST : public MValueAST {
public:
    IntegerAST(uint64_t val);

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    virtual std::string toString() const;

private:
    uint64_t val;
};
class StringAST : public MValueAST {
public:
    StringAST(std::string val);

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    virtual std::string toString() const;

private:
    std::string val;
};

class CondExpr : public MValueAST {
public:
    CondExpr(MValueAST *cond, MValueAST *thenVal, MValueAST *elseVal):
        cond(cond),
        thenVal(thenVal),
        elseVal(elseVal)
    {}

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    virtual std::string toString() const;

private:
    MValueAST *cond, *thenVal, *elseVal;
};

typedef std::vector<MValueAST*> MValueList;
typedef std::map<std::string,MValueAST*> MValueMap;


class CallExpr : public MValueAST {
public:
    CallExpr( MValueAST *val, TupleAST *args ):
        val(val),
        args(args)
    {}

    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    std::string toString() const;

private:
    MValueAST *val;
    TupleAST *args;
};

class Statement;
struct StatementList : std::vector<Statement*> {
};

class Statement {
public:
    virtual void codegen(Context *ctx) = 0;

    virtual void print(Printer &p) const = 0;
    friend Printer& operator << ( Printer &p, const Statement &stmt );

    virtual void collectSystemDecl(Context *ctx) const {};
};

class TypeDecl : public Statement {
public:
    TypeDecl( std::string name, MTypeAST *type ):
        name(name),
        type(type)
    {}

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;
private:
    std::string name;
    MTypeAST *type;
};

class FunctionDecl : public Statement {
public:
    FunctionDecl( std::string name, MTupleTypeAST *args, StatementList *list, MTypeAST *retType ):
        name(name),
        args(args),
        stmts(list),
        retType(retType)
    {}

    virtual void codegen(Context *ctx);

    virtual void print(Printer &p) const;
private:
    std::string name;
    MTupleTypeAST *args;
    StatementList *stmts;
    MTypeAST *retType;
};

class VarDecl : public Statement {
public:
    VarDecl(std::string name, MTypeAST *type, MValueAST *val);
    virtual void codegen(Context *ctx);
    virtual void collectSystemDecl(Context *ctx) const;
    virtual void print(Printer &p) const;

private:
    std::string name;
    MTypeAST *type;
    MValueAST *val;
};

class ImportStmt : public Statement {
public:
    ImportStmt(Path from);

    virtual void codegen(Context *ctx);
};

class BindStmt : public Statement {
public:
    BindStmt(Path target, MValueAST *value):
        target(target),
        value(value)
    {}

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;

private:
    // TODO: not path, but expression
    Path target;
    MValueAST *value;
};

typedef std::vector<std::pair<std::string, MValueAST*>> StringValueVector;
class MatchAssignStmt : public Statement {
public:
    MatchAssignStmt(Path target, MValueAST *src, StringValueVector cases):
        target(target),
        src(src),
        cases(cases)
    {}

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;

private:
    Statement* lower();

private:
    Path target;
    MValueAST *src;
    StringValueVector cases;

    Statement *_lower = 0;
};

typedef std::vector<std::pair<MValueAST*,StatementList*>> CondStmtList;

class CondStmt : public Statement { 
public:
    CondStmt(CondStmtList stmts, StatementList *elStmt);

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;
private:
    CondStmtList stmts;
    StatementList *elStmt;
};

class ReturnStmt : public Statement {
public:
    ReturnStmt(MValueAST *val);

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;
private:
    MValueAST *val;
};

class ForStmt : public Statement {
public:
    ForStmt(std::string target_name, MValueAST *inval, StatementList *stmts):
        target_name(target_name),
        inval(inval),
        stmts(stmts)
    {}

    virtual void codegen(Context *ctx);
    virtual void print(Printer &p) const;
private:
    std::string target_name;
    MValueAST *inval;
    StatementList *stmts;
};


#endif // HCLANG_AST_BASE_H
