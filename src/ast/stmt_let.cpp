#include "stmt_let.h"

#include "printer.h"

void LetStmt::codegen(Context *ctx) {
    MValueType *t = 0;
    if( letType )
        t = letType->codegen(ctx);

    MValue *val;
    val = value->codegen(ctx, t);
    ctx->bindValue(target[0],val); // TODO
}

void LetStmt::print(Printer &p) const {
    p.println( target[0] + " = " + value->toString()); // TODO
}
