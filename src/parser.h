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
};

#endif // HCLANG_PARSER_H

