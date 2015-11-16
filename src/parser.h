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
 *
 */
#ifndef HCLANG_PARSER_H
#define HCLANG_PARSER_H

#include <string>
#include "lexer.h"

#include "ast/ast.h"

class Parser {
    const std::vector<Token> &tokens;
    int _current_idx;
    std::ostream *out = 0;

public:
    Parser( const std::vector<Token> &tokens, std::ostream *out = 0 );

    void errorExpansion(std::string what);
    bool expect(Token::Type type);
    bool expectConsume(Token::Type type);
    bool tryConsume(Token::Type type);
    void consume();
    Token::Type type(int ahead = 0);
    const Token& current(int ahead = 0);
    const Token& currentConsume();

    StatementList* file();

    Statement*  statement();
    VarDecl*    varStatement();

    Statement*  pathPrefixStatement();
    Statement*   bindStatement(Path p);
    Statement*  letStatement();
    SendStmt*   sendStatement(Path p);

    SystemDecl*     systemDecl();
    FunctionDecl*   functionDecl();
    ProcedureDecl*  procedureDecl();
    SlotDecl*       slotDecl();
    CondStmt*       condStatement();
    TypeDecl*       typeStatement();
    ForStmt*        forStatement();
    MatchAssignStmt*    matchAssignStatement(Path path);
    ReturnStmt*     returnStatement();

    MValueAST*       value();
    MValueAST*      comparison();
    MValueAST*       plus();
    MValueAST*       mult();
    MValueAST*       atom();
    MValueAST*       atom_expr();
    MValueAST*      condExpr();
    ArrayAST*       array();
    CallExpr*       call();
    TupleAST*       tuple();
    //BinaryExpr*     binExpr();
    FunctionDecl*   shortFunctionDecl();
    SpawnExpr*      spawnExpr();


    MTupleTypeAST*  argsDecl();
    MTypeAST*       typeDecl();
    MTypeAST*       unionTypeDecl();
    MTypeAST*       baseTypeDecl();
    MTupleTypeAST*       tupleTypeDecl();
    MTypeAST*       arrayTypeTrailer(MTypeAST  *type);

    Path            path();
    SlotTypeAST*    slotTypeDecl();
};

#endif // HCLANG_PARSER_H

