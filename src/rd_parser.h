#pragma once
#include "token.h"
#include "lexer.h"
#include "symbol_table.h"
#include "error_handler.h"
#include <vector>
#include <set>

class RDParser {
public:
    RDParser(const std::vector<Token>& tokens,
             SymbolTableManager& st,
             ErrorHandler& eh);
    bool parse();

private:
    std::vector<Token> tokens;
    int pos;
    SymbolTableManager& symTable;
    ErrorHandler& errHandler;

    Token& current();
    Token  consume();
    bool   check(TokenType t) const;
    bool   match(TokenType t);
    bool   expect(TokenType t, const std::string& what);
    void   synchronize(const std::initializer_list<TokenType>& follow);
    bool   isTypeToken() const;

    // Program structure
    void parseProgram();
    void parseDecl();
    void parseClassDecl();
    void parseInterfaceDecl();
    void parsePrototype();
    void parseField();
    void parseVarDecl(const std::string& type, const std::string& name, int line);
    void parseFuncDecl(const std::string& retType,
                       const std::string& name, int line, bool isStatic);
    void parseFormals();

    // Types
    std::string parseType();

    // Statements
    void parseStmtBlock();
    void parseStmt();
    void parseIfStmt();
    void parseWhileStmt();
    void parseForStmt();
    void parseReturnStmt();
    void parseBreakStmt();
    void parsePrintStmt();

    // Expressions (precedence climbing)
    void parseExpr();
    void parseAssign();
    void parseOr();
    void parseAnd();
    void parseEquality();
    void parseRelational();
    void parseAdditive();
    void parseMultiplicative();
    void parseUnary();
    void parsePostfix();
    void parsePrimary();
    void parseActuals();
};