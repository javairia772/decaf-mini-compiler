#include "rd_parser.h"
#include <iostream>

RDParser::RDParser(const std::vector<Token>& toks,
                   SymbolTableManager& st, ErrorHandler& eh)
    : tokens(toks), pos(0), symTable(st), errHandler(eh) {}

Token& RDParser::current() { return tokens[pos]; }

Token RDParser::consume() {
    Token t = tokens[pos];
    if (pos < (int)tokens.size() - 1) pos++;
    return t;
}

bool RDParser::check(TokenType t) const { return tokens[pos].type == t; }

bool RDParser::match(TokenType t) {
    if (check(t)) { consume(); return true; }
    return false;
}

bool RDParser::expect(TokenType t, const std::string& what) {
    if (check(t)) { consume(); return true; }
    errHandler.reportError(ErrorType::SYNTACTIC,
        current().line, current().col,
        "Expected " + what + ", got '" + current().lexeme + "'");
    return false;
}

void RDParser::synchronize(const std::initializer_list<TokenType>& follow) {
    std::set<TokenType> fs(follow);
    while (!check(TokenType::EOF_TOK)) {
        if (fs.count(current().type)) return;
        consume();
    }
}

bool RDParser::isTypeToken() const {
    TokenType t = tokens[pos].type;
    return t == TokenType::INT    || t == TokenType::DOUBLE ||
           t == TokenType::BOOL   || t == TokenType::STRING ||
           t == TokenType::VOID   || t == TokenType::IDENTIFIER;
}

// ─── Program ────────────────────────────────────────────────────────────────

bool RDParser::parse() {
    std::cout << "\n[RD Parser] Starting recursive descent parse...\n";
    parseProgram();
    bool ok = !errHandler.hasErrors();
    std::cout << "[RD Parser] " << (ok ? "SUCCESS" : "FAILED") << "\n";
    return ok;
}

void RDParser::parseProgram() {
    while (!check(TokenType::EOF_TOK)) {
        if (check(TokenType::CLASS) || check(TokenType::INTERFACE))
            parseDecl();
        else {
            errHandler.reportError(ErrorType::SYNTACTIC,
                current().line, current().col,
                "Expected class or interface declaration");
            synchronize({TokenType::CLASS,
                         TokenType::INTERFACE,
                         TokenType::EOF_TOK});
        }
    }
}

void RDParser::parseDecl() {
    if (check(TokenType::CLASS))     parseClassDecl();
    else if (check(TokenType::INTERFACE)) parseInterfaceDecl();
}

// ─── Class ──────────────────────────────────────────────────────────────────

void RDParser::parseClassDecl() {
    expect(TokenType::CLASS, "class");
    std::string name = current().lexeme;
    int ln = current().line;
    expect(TokenType::IDENTIFIER, "class name");

    symTable.declare({name, SymbolKind::CLASS, "class",
                      symTable.currentLevel(), ln, ""});
    symTable.enterScope();

    // extends
    if (match(TokenType::EXTENDS))
        expect(TokenType::IDENTIFIER, "superclass name");

    // implements id, id, ...
    if (match(TokenType::IMPLEMENTS)) {
        expect(TokenType::IDENTIFIER, "interface name");
        while (match(TokenType::COMMA))
            expect(TokenType::IDENTIFIER, "interface name");
    }

    expect(TokenType::LBRACE, "{");

    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOK))
        parseField();

    expect(TokenType::RBRACE, "}");
    symTable.exitScope();
}

// ─── Interface ──────────────────────────────────────────────────────────────

void RDParser::parseInterfaceDecl() {
    expect(TokenType::INTERFACE, "interface");
    std::string name = current().lexeme;
    int ln = current().line;
    expect(TokenType::IDENTIFIER, "interface name");

    symTable.declare({name, SymbolKind::CLASS, "interface",
                      symTable.currentLevel(), ln, ""});
    symTable.enterScope();
    expect(TokenType::LBRACE, "{");

    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOK))
        parsePrototype();

    expect(TokenType::RBRACE, "}");
    symTable.exitScope();
}

void RDParser::parsePrototype() {
    std::string retType = parseType();
    std::string name = current().lexeme;
    int ln = current().line;
    expect(TokenType::IDENTIFIER, "method name");
    symTable.declare({name, SymbolKind::FUNCTION, retType,
                      symTable.currentLevel(), ln, "interface"});
    symTable.enterScope();
    expect(TokenType::LPAREN, "(");
    if (!check(TokenType::RPAREN)) parseFormals();
    expect(TokenType::RPAREN, ")");
    symTable.exitScope();
    expect(TokenType::SEMICOLON, ";");
}

// ─── Fields (vars + methods) ────────────────────────────────────────────────

void RDParser::parseField() {
    bool isStatic = match(TokenType::STATIC);
    (void)isStatic; // stored in extra below

    if (!isTypeToken()) {
        errHandler.reportError(ErrorType::SYNTACTIC,
            current().line, current().col,
            "Expected field declaration");
        synchronize({TokenType::RBRACE, TokenType::EOF_TOK});
        return;
    }

    std::string type = parseType();
    std::string name = current().lexeme;
    int ln = current().line;
    expect(TokenType::IDENTIFIER, "field name");

    if (check(TokenType::LPAREN))
        parseFuncDecl(type, name, ln, isStatic);
    else
        parseVarDecl(type, name, ln);
}

void RDParser::parseVarDecl(const std::string& type,
                             const std::string& name, int line) {
    symTable.declare({name, SymbolKind::VARIABLE, type,
                      symTable.currentLevel(), line, ""});
    // multiple declarators: int x, y, z;
    while (match(TokenType::COMMA)) {
        std::string n = current().lexeme;
        int l = current().line;
        expect(TokenType::IDENTIFIER, "variable name");
        symTable.declare({n, SymbolKind::VARIABLE, type,
                          symTable.currentLevel(), l, ""});
    }
    expect(TokenType::SEMICOLON, ";");
}

void RDParser::parseFuncDecl(const std::string& retType,
                              const std::string& name,
                              int line, bool isStatic) {
    symTable.declare({name, SymbolKind::FUNCTION, retType,
                      symTable.currentLevel(), line,
                      isStatic ? "static" : ""});
    symTable.enterScope();
    expect(TokenType::LPAREN, "(");
    if (!check(TokenType::RPAREN)) parseFormals();
    expect(TokenType::RPAREN, ")");
    parseStmtBlock();
    symTable.exitScope();
}

void RDParser::parseFormals() {
    // first formal
    std::string type = parseType();
    std::string name = current().lexeme;
    int ln = current().line;
    expect(TokenType::IDENTIFIER, "parameter name");
    symTable.declare({name, SymbolKind::PARAMETER, type,
                      symTable.currentLevel(), ln, ""});
    while (match(TokenType::COMMA)) {
        type = parseType();
        name = current().lexeme;
        ln   = current().line;
        expect(TokenType::IDENTIFIER, "parameter name");
        symTable.declare({name, SymbolKind::PARAMETER, type,
                          symTable.currentLevel(), ln, ""});
    }
}

// ─── Types ──────────────────────────────────────────────────────────────────

std::string RDParser::parseType() {
    std::string base;
    switch (current().type) {
        case TokenType::INT:        base = "int";    consume(); break;
        case TokenType::DOUBLE:     base = "double"; consume(); break;
        case TokenType::BOOL:       base = "bool";   consume(); break;
        case TokenType::STRING:     base = "string"; consume(); break;
        case TokenType::VOID:       base = "void";   consume(); break;
        case TokenType::IDENTIFIER: base = current().lexeme; consume(); break;
        default:
            errHandler.reportError(ErrorType::SYNTACTIC,
                current().line, current().col,
                "Expected type, got '" + current().lexeme + "'");
            return "?";
    }
    // array: int[]  or  int[][]
    while (check(TokenType::LBRACKET) &&
           tokens[pos+1].type == TokenType::RBRACKET) {
        consume(); consume();
        base += "[]";
    }
    return base;
}

// ─── Statements ─────────────────────────────────────────────────────────────

void RDParser::parseStmtBlock() {
    expect(TokenType::LBRACE, "{");
    symTable.enterScope();

    // local variable declarations first
    while (isTypeToken() &&
           tokens[pos+1].type == TokenType::IDENTIFIER &&
           tokens[pos+2].type != TokenType::LPAREN) {
        std::string type = parseType();
        std::string name = current().lexeme;
        int ln = current().line;
        expect(TokenType::IDENTIFIER, "variable name");
        parseVarDecl(type, name, ln);
    }

    // then statements
    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOK))
        parseStmt();

    symTable.exitScope();
    expect(TokenType::RBRACE, "}");
}

void RDParser::parseStmt() {
    switch (current().type) {
        case TokenType::IF:        parseIfStmt();     break;
        case TokenType::WHILE:     parseWhileStmt();  break;
        case TokenType::FOR:       parseForStmt();    break;
        case TokenType::RETURN:    parseReturnStmt(); break;
        case TokenType::BREAK:     parseBreakStmt();  break;
        case TokenType::PRINT:     parsePrintStmt();  break;
        case TokenType::LBRACE:    parseStmtBlock();  break;
        case TokenType::SEMICOLON: consume();         break; // empty stmt
        default:
            // expression statement
            parseExpr();
            expect(TokenType::SEMICOLON, ";");
            break;
    }
}

void RDParser::parseIfStmt() {
    expect(TokenType::IF, "if");
    expect(TokenType::LPAREN, "(");
    parseExpr();
    expect(TokenType::RPAREN, ")");
    parseStmt();
    if (match(TokenType::ELSE)) parseStmt();
}

void RDParser::parseWhileStmt() {
    expect(TokenType::WHILE, "while");
    expect(TokenType::LPAREN, "(");
    parseExpr();
    expect(TokenType::RPAREN, ")");
    parseStmt();
}

void RDParser::parseForStmt() {
    expect(TokenType::FOR, "for");
    expect(TokenType::LPAREN, "(");
    // init
    if (!check(TokenType::SEMICOLON)) parseExpr();
    expect(TokenType::SEMICOLON, ";");
    // condition
    if (!check(TokenType::SEMICOLON)) parseExpr();
    expect(TokenType::SEMICOLON, ";");
    // update
    if (!check(TokenType::RPAREN)) parseExpr();
    expect(TokenType::RPAREN, ")");
    parseStmt();
}

void RDParser::parseReturnStmt() {
    expect(TokenType::RETURN, "return");
    if (!check(TokenType::SEMICOLON)) parseExpr();
    expect(TokenType::SEMICOLON, ";");
}

void RDParser::parseBreakStmt() {
    expect(TokenType::BREAK, "break");
    expect(TokenType::SEMICOLON, ";");
}

void RDParser::parsePrintStmt() {
    expect(TokenType::PRINT, "Print");
    expect(TokenType::LPAREN, "(");
    parseExpr();
    while (match(TokenType::COMMA)) parseExpr();
    expect(TokenType::RPAREN, ")");
    expect(TokenType::SEMICOLON, ";");
}

// ─── Expressions (full precedence climbing) ─────────────────────────────────

void RDParser::parseExpr()             { parseAssign(); }

void RDParser::parseAssign() {
    parseOr();
    if (match(TokenType::ASSIGN)) parseAssign(); // right-assoc
}

void RDParser::parseOr() {
    parseAnd();
    while (match(TokenType::OR)) parseAnd();
}

void RDParser::parseAnd() {
    parseEquality();
    while (match(TokenType::AND)) parseEquality();
}

void RDParser::parseEquality() {
    parseRelational();
    while (check(TokenType::EQ) || check(TokenType::NEQ)) {
        consume(); parseRelational();
    }
}

void RDParser::parseRelational() {
    parseAdditive();
    while (check(TokenType::LT)  || check(TokenType::LTE) ||
           check(TokenType::GT)  || check(TokenType::GTE)) {
        consume(); parseAdditive();
    }
}

void RDParser::parseAdditive() {
    parseMultiplicative();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        consume(); parseMultiplicative();
    }
}

void RDParser::parseMultiplicative() {
    parseUnary();
    while (check(TokenType::STAR)  ||
           check(TokenType::SLASH) ||
           check(TokenType::PERCENT)) {
        consume(); parseUnary();
    }
}

void RDParser::parseUnary() {
    if (check(TokenType::NOT) || check(TokenType::MINUS)) {
        consume(); parseUnary();
    } else {
        parsePostfix();
    }
}

void RDParser::parsePostfix() {
    parsePrimary();
    while (true) {
        if (check(TokenType::LBRACKET)) {
            // array index: expr[expr]
            consume();
            parseExpr();
            expect(TokenType::RBRACKET, "]");
        } else if (check(TokenType::DOT)) {
            consume();
            std::string field = current().lexeme;
            expect(TokenType::IDENTIFIER, "field name");
            // method call: expr.id(actuals)
            if (check(TokenType::LPAREN)) {
                consume();
                parseActuals();
                expect(TokenType::RPAREN, ")");
            }
            (void)field;
        } else {
            break;
        }
    }
}

void RDParser::parsePrimary() {
    switch (current().type) {
        case TokenType::INT_LITERAL:
        case TokenType::DOUBLE_LITERAL:
        case TokenType::STRING_LITERAL:
        case TokenType::TRUE_TOK:
        case TokenType::FALSE_TOK:
        case TokenType::NULL_TOK:
            consume();
            break;

        case TokenType::THIS:
            consume();
            break;

        case TokenType::IDENTIFIER: {
            std::string name = current().lexeme;
            int ln = current().line, cl = current().col;
            consume();
            if (check(TokenType::LPAREN)) {
                // function call: id(actuals)
                consume();
                parseActuals();
                expect(TokenType::RPAREN, ")");
            } else {
                // variable use — semantic check
                if (!symTable.resolve(name))
                    errHandler.reportError(ErrorType::SEMANTIC, ln, cl,
                        "Undeclared identifier '" + name + "'");
            }
            break;
        }

        case TokenType::LPAREN:
            consume();
            parseExpr();
            expect(TokenType::RPAREN, ")");
            break;

        case TokenType::NEW:
            consume();
            if (check(TokenType::IDENTIFIER)) {
                // new ClassName()
                consume();
                expect(TokenType::LPAREN, "(");
                expect(TokenType::RPAREN, ")");
            } else {
                // new Type[Expr]
                parseType();
                expect(TokenType::LBRACKET, "[");
                parseExpr();
                expect(TokenType::RBRACKET, "]");
            }
            break;

        default:
            errHandler.reportError(ErrorType::SYNTACTIC,
                current().line, current().col,
                "Unexpected token '" + current().lexeme + "' in expression");
            consume();
            break;
    }
}

void RDParser::parseActuals() {
    if (check(TokenType::RPAREN)) return;
    parseExpr();
    while (match(TokenType::COMMA)) parseExpr();
}