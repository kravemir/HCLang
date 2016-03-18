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
#ifndef HCLANG_AST_TYPES_STRING_H
#define HCLANG_AST_TYPES_STRING_H

#include <ast/base.h>

struct MethodReferenceType : MValueType {
    static MethodReferenceType* create( MValueType *retType, llvm::FunctionType *ft ) {
        llvm::Type* lt = llvm::StructType::create({llvm::Type::getInt8PtrTy( lctx ),ft});
        return new MethodReferenceType(lt,retType);
    }

    MValue* createValue(MValue *src, llvm::Value *functionPtr) {
        llvm::Value *val_with_instance = Builder.CreateInsertValue(
                llvm::UndefValue::get(llvmType()),
                src->value(),
                { 0 },
                "methodref.instance"
        );
        llvm::Value *val_with_method = Builder.CreateInsertValue(
                val_with_instance,
                functionPtr,
                { 1 },
                "methodref.method"
        );
        return new MValue(this,val_with_method,false);
    }


private:
    MethodReferenceType(llvm::Type* _llvmType, MValueType *retType):
            MValueType(_llvmType,true,retType)
    {}
};

struct StringType : MValueType {
    llvm::Function *F;

    static StringType* create(Context *ctx);

    virtual MValue* createCast ( Context* ctx, MValue* src );
    virtual MValue* getChild(MValue *src, std::string name);
    virtual MValueType* getChildType(std::string name);

private:
    StringType(llvm::Type* type, llvm::Function *F) : MValueType(type), F(F) { assert(type); };
};

class StringAST : public MValueAST {
public:
    StringAST(std::string val);

    virtual MValueType* calculateType(Context *ctx) {
        return StringType::create(ctx);
    };
    virtual MValue* codegen(Context *ctx, MValueType *type = 0);
    virtual std::string toString() const;

private:
    std::string val;
};

#endif //HCLANG_AST_TYPES_STRING_H
