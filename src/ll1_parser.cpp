#include "ll1_parser.h"
#include <iostream>
#include <iomanip>
#include <stack>
#include <algorithm>

// ─── Constructor ─────────────────────────────────────────────────────────────

LL1Parser::LL1Parser(const std::vector<Token>& toks, ErrorHandler& eh)
    : tokens(toks), pos(0), errHandler(eh) {
    buildGrammar();
    computeFirstSets();
    computeFollowSets();
    buildTable();
}

// ─── Grammar Definition ──────────────────────────────────────────────────────
// We use a simplified-but-complete LL(1)-compatible Decaf grammar.
// Left recursion is removed. Left factoring is applied.
// Symbols starting with uppercase = NonTerminals
// All others = terminals matching TokenType::typeToString()

void LL1Parser::buildGrammar() {
    // Program -> Decl Program | ε
    grammar["Program"] = {
        {"Decl", "Program"},
        {} // ε
    };
    // Decl -> ClassDecl | InterfaceDecl
    grammar["Decl"] = {
        {"ClassDecl"},
        {"InterfaceDecl"}
    };
    // ClassDecl -> CLASS IDENTIFIER OptExtends OptImplements LBRACE FieldList RBRACE
    grammar["ClassDecl"] = {
        {"CLASS","IDENTIFIER","OptExtends","OptImplements",
         "LBRACE","FieldList","RBRACE"}
    };
    grammar["OptExtends"] = {
        {"EXTENDS","IDENTIFIER"},
        {} // ε
    };
    grammar["OptImplements"] = {
        {"IMPLEMENTS","IDENTIFIER","MoreInterfaces"},
        {} // ε
    };
    grammar["MoreInterfaces"] = {
        {"COMMA","IDENTIFIER","MoreInterfaces"},
        {} // ε
    };
    // InterfaceDecl -> INTERFACE IDENTIFIER LBRACE ProtoList RBRACE
    grammar["InterfaceDecl"] = {
        {"INTERFACE","IDENTIFIER","LBRACE","ProtoList","RBRACE"}
    };
    grammar["ProtoList"] = {
        {"Proto","ProtoList"},
        {} // ε
    };
    grammar["Proto"] = {
        {"Type","IDENTIFIER","LPAREN","OptFormals","RPAREN","SEMICOLON"}
    };
    // FieldList -> Field FieldList | ε
    grammar["FieldList"] = {
        {"Field","FieldList"},
        {} // ε
    };
    // Field -> OptStatic Type IDENTIFIER FieldRest
    grammar["Field"] = {
        {"OptStatic","Type","IDENTIFIER","FieldRest"}
    };
    grammar["OptStatic"] = {
        {"STATIC"},
        {} // ε
    };
    // FieldRest -> SEMICOLON (var) | LPAREN OptFormals RPAREN StmtBlock (func)
    grammar["FieldRest"] = {
        {"SEMICOLON"},
        {"LPAREN","OptFormals","RPAREN","StmtBlock"}
    };
    // Type -> BaseType OptArray
    grammar["Type"] = {
        {"BaseType","OptArray"}
    };
    grammar["BaseType"] = {
        {"INT"}, {"DOUBLE"}, {"BOOL"}, {"STRING"}, {"VOID"}, {"IDENTIFIER"}
    };
    grammar["OptArray"] = {
        {"LBRACKET","RBRACKET","OptArray"},
        {} // ε
    };
    // Formals
    grammar["OptFormals"] = {
        {"Formals"},
        {} // ε
    };
    grammar["Formals"] = {
        {"Type","IDENTIFIER","MoreFormals"}
    };
    grammar["MoreFormals"] = {
        {"COMMA","Type","IDENTIFIER","MoreFormals"},
        {} // ε
    };
    // StmtBlock -> LBRACE VarDeclList StmtList RBRACE
    grammar["StmtBlock"] = {
        {"LBRACE","LocalVarList","StmtList","RBRACE"}
    };
    grammar["LocalVarList"] = {
        {"Type","IDENTIFIER","SEMICOLON","LocalVarList"},
        {} // ε
    };
    grammar["StmtList"] = {
        {"Stmt","StmtList"},
        {} // ε
    };
    // Stmt
    grammar["Stmt"] = {
        {"IfStmt"},
        {"WhileStmt"},
        {"ForStmt"},
        {"ReturnStmt"},
        {"BreakStmt"},
        {"PrintStmt"},
        {"StmtBlock"},
        {"ExprStmt"}
    };
    grammar["ExprStmt"] = {
        {"Expr","SEMICOLON"},
        {"SEMICOLON"} // empty statement
    };
    grammar["IfStmt"] = {
        {"IF","LPAREN","Expr","RPAREN","Stmt","OptElse"}
    };
    grammar["OptElse"] = {
        {"ELSE","Stmt"},
        {} // ε
    };
    grammar["WhileStmt"] = {
        {"WHILE","LPAREN","Expr","RPAREN","Stmt"}
    };
    grammar["ForStmt"] = {
        {"FOR","LPAREN","OptExpr","SEMICOLON",
         "OptExpr","SEMICOLON","OptExpr","RPAREN","Stmt"}
    };
    grammar["ReturnStmt"] = {
        {"RETURN","OptExpr","SEMICOLON"}
    };
    grammar["BreakStmt"] = {
        {"BREAK","SEMICOLON"}
    };
    grammar["PrintStmt"] = {
        {"PRINT","LPAREN","Expr","MorePrintArgs","RPAREN","SEMICOLON"}
    };
    grammar["MorePrintArgs"] = {
        {"COMMA","Expr","MorePrintArgs"},
        {} // ε
    };
    grammar["OptExpr"] = {
        {"Expr"},
        {} // ε
    };
    // Expressions — left recursion removed via right-recursive Expr'
    // Expr -> UnaryExpr ExprRest
    grammar["Expr"] = {
        {"UnaryExpr","ExprRest"}
    };
    // ExprRest -> AssignOp Expr | BinOp UnaryExpr ExprRest | ε
    grammar["ExprRest"] = {
        {"ASSIGN","Expr"},
        {"OR","UnaryExpr","ExprRest"},
        {"AND","UnaryExpr","ExprRest"},
        {"EQ","UnaryExpr","ExprRest"},
        {"NEQ","UnaryExpr","ExprRest"},
        {"LT","UnaryExpr","ExprRest"},
        {"LTE","UnaryExpr","ExprRest"},
        {"GT","UnaryExpr","ExprRest"},
        {"GTE","UnaryExpr","ExprRest"},
        {"PLUS","UnaryExpr","ExprRest"},
        {"MINUS","UnaryExpr","ExprRest"},
        {"STAR","UnaryExpr","ExprRest"},
        {"SLASH","UnaryExpr","ExprRest"},
        {"PERCENT","UnaryExpr","ExprRest"},
        {} // ε
    };
    grammar["UnaryExpr"] = {
        {"NOT","UnaryExpr"},
        {"MINUS","UnaryExpr"},
        {"PostfixExpr"}
    };
    grammar["PostfixExpr"] = {
        {"Primary","PostfixRest"}
    };
    grammar["PostfixRest"] = {
        {"LBRACKET","Expr","RBRACKET","PostfixRest"},
        {"DOT","IDENTIFIER","OptCall","PostfixRest"},
        {} // ε
    };
    grammar["OptCall"] = {
        {"LPAREN","Actuals","RPAREN"},
        {} // ε
    };
    grammar["Primary"] = {
        {"INT_LITERAL"},
        {"DOUBLE_LITERAL"},
        {"STRING_LITERAL"},
        {"TRUE"},
        {"FALSE"},
        {"NULL"},
        {"THIS"},
        {"LPAREN","Expr","RPAREN"},
        {"NEW","NewRest"},
        {"IDENTIFIER","IdRest"}
    };
    grammar["NewRest"] = {
        {"IDENTIFIER","LPAREN","RPAREN"},
        {"BaseType","LBRACKET","Expr","RBRACKET"}
    };
    grammar["IdRest"] = {
        {"LPAREN","Actuals","RPAREN"}, // function call
        {} // ε — plain identifier
    };
    grammar["Actuals"] = {
        {"Expr","MoreActuals"},
        {} // ε
    };
    grammar["MoreActuals"] = {
        {"COMMA","Expr","MoreActuals"},
        {} // ε
    };

    // Collect all NT names
    for (auto& [nt, _] : grammar)
        nonTerminals.insert(nt);

    // Collect all terminals (anything not an NT and not "ε")
    for (auto& [nt, prods] : grammar)
        for (auto& prod : prods)
            for (auto& sym : prod)
                if (!nonTerminals.count(sym) && sym != "EPS")
                    terminals.insert(sym);
    terminals.insert("$");
}

// ─── FIRST sets ───────────────────────────────────────────────────────────────

bool LL1Parser::isNonTerminal(const std::string& s) const {
    return nonTerminals.count(s) > 0;
}
bool LL1Parser::isTerminal(const std::string& s) const {
    return !isNonTerminal(s);
}

std::set<std::string> LL1Parser::firstOfSequence(
        const std::vector<std::string>& seq) const {
    std::set<std::string> result;
    bool allHaveEps = true;
    for (auto& sym : seq) {
        if (isTerminal(sym)) {
            result.insert(sym);
            allHaveEps = false;
            break;
        }
        auto it = FIRST.find(sym);
        if (it == FIRST.end()) { allHaveEps = false; break; }
        for (auto& t : it->second)
            if (t != "EPS") result.insert(t);
        if (!it->second.count("EPS")) { allHaveEps = false; break; }
    }
    if (allHaveEps) result.insert("EPS");
    return result;
}

void LL1Parser::computeFirstSets() {
    // Initialize
    for (auto& nt : nonTerminals) FIRST[nt] = {};

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& [nt, prods] : grammar) {
            for (auto& prod : prods) {
                std::set<std::string> add;
                if (prod.empty()) {
                    add.insert("EPS");
                } else {
                    add = firstOfSequence(prod);
                }
                for (auto& s : add) {
                    if (!FIRST[nt].count(s)) {
                        FIRST[nt].insert(s);
                        changed = true;
                    }
                }
            }
        }
    }
}

// ─── FOLLOW sets ──────────────────────────────────────────────────────────────

void LL1Parser::computeFollowSets() {
    for (auto& nt : nonTerminals) FOLLOW[nt] = {};
    FOLLOW["Program"].insert("$");

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& [nt, prods] : grammar) {
            for (auto& prod : prods) {
                for (int i = 0; i < (int)prod.size(); i++) {
                    if (!isNonTerminal(prod[i])) continue;
                    std::string B = prod[i];
                    // FIRST of what follows B in this production
                    std::vector<std::string> beta(prod.begin()+i+1, prod.end());
                    auto fb = firstOfSequence(beta);
                    for (auto& t : fb) {
                        if (t != "EPS" && !FOLLOW[B].count(t)) {
                            FOLLOW[B].insert(t);
                            changed = true;
                        }
                    }
                    if (fb.count("EPS") || beta.empty()) {
                        for (auto& t : FOLLOW[nt]) {
                            if (!FOLLOW[B].count(t)) {
                                FOLLOW[B].insert(t);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

// ─── LL(1) Table ─────────────────────────────────────────────────────────────

void LL1Parser::buildTable() {
    for (auto& [nt, prods] : grammar) {
        for (auto& prod : prods) {
            std::set<std::string> first;
            if (prod.empty())
                first.insert("EPS");
            else
                first = firstOfSequence(prod);

            for (auto& t : first) {
                if (t != "EPS")
                    table[{nt, t}] = prod;
            }
            if (first.count("EPS")) {
                for (auto& t : FOLLOW[nt])
                    table[{nt, t}] = prod;
            }
        }
    }
}

// ─── Helpers ─────────────────────────────────────────────────────────────────

Token& LL1Parser::current() { return tokens[pos]; }
Token  LL1Parser::consume() {
    Token t = tokens[pos];
    if (pos < (int)tokens.size()-1) pos++;
    return t;
}
bool LL1Parser::check(TokenType t) const { return tokens[pos].type == t; }

std::string LL1Parser::tokenToTerminal(const Token& t) const {
    if (t.type == TokenType::EOF_TOK)    return "$";
    if (t.type == TokenType::TRUE_TOK)   return "TRUE";
    if (t.type == TokenType::FALSE_TOK)  return "FALSE";
    if (t.type == TokenType::NULL_TOK)   return "NULL";
    return t.typeToString();
}

// ─── Parse ───────────────────────────────────────────────────────────────────

bool LL1Parser::parse() {
    std::cout << "\n[LL(1) Parser] Starting table-driven parse...\n";
    std::cout << std::string(95, '-') << "\n";
    std::cout << std::left
              << std::setw(38) << "STACK TOP"
              << std::setw(28) << "INPUT"
              << "ACTION\n";
    std::cout << std::string(95, '-') << "\n";

    std::stack<std::string> stk;
    stk.push("$");
    stk.push("Program");

    auto inputPreview = [&]() {
        std::string s;
        for (int i = pos; i < std::min((int)tokens.size(), pos+3); i++)
            s += tokens[i].lexeme + " ";
        return s;
    };

    bool success = true;

    while (!stk.empty()) {
        std::string top = stk.top();
        std::string inp = tokenToTerminal(current());

        if (top == "$") {
            if (inp == "$") {
                std::cout << std::left << std::setw(38) << "$"
                          << std::setw(28) << "$"
                          << "ACCEPT\n";
                break;
            } else {
                std::cout << std::left << std::setw(38) << "$"
                          << std::setw(28) << inputPreview()
                          << "ERROR: extra tokens\n";
                success = false; break;
            }
        }

        if (isTerminal(top)) {
            if (top == inp) {
                std::cout << std::left << std::setw(38) << top
                          << std::setw(28) << inputPreview()
                          << "MATCH " + top + "\n";
                stk.pop(); consume();
            } else {
                errHandler.reportError(ErrorType::SYNTACTIC,
                    current().line, current().col,
                    "Expected '" + top + "', got '" + current().lexeme + "'");
                std::cout << std::left << std::setw(38) << top
                          << std::setw(28) << inputPreview()
                          << "ERROR: mismatch — skip\n";
                success = false;
                // panic: pop terminal, try to continue
                stk.pop();
            }
            continue;
        }

        // Non-terminal
        auto key = std::make_pair(top, inp);
        if (table.count(key)) {
            auto& prod = table.at(key);
            std::string rhs;
            for (auto& s : prod) rhs += s + " ";
            if (prod.empty()) rhs = "ε";
            std::cout << std::left << std::setw(38) << top
                      << std::setw(28) << inputPreview()
                      << top + " -> " + rhs + "\n";
            stk.pop();
            for (auto it = prod.rbegin(); it != prod.rend(); ++it)
                stk.push(*it);
        } else {
            errHandler.reportError(ErrorType::SYNTACTIC,
                current().line, current().col,
                "LL(1): No rule for (" + top + ", " + inp + ")");
            std::cout << std::left << std::setw(38) << top
                      << std::setw(28) << inputPreview()
                      << "ERROR: panic — skip '" + current().lexeme + "'\n";
            success = false;
            // panic-mode: use FOLLOW to synchronize
            if (FOLLOW.count(top)) {
                auto& fol = FOLLOW.at(top);
                while (inp != "$" && !fol.count(inp)) {
                    consume();
                    inp = tokenToTerminal(current());
                }
                stk.pop();
            } else {
                consume();
            }
        }
    }

    std::cout << "[LL(1) Parser] "
              << (success ? "SUCCESS" : "FAILED") << "\n";
    return success;
}

// ─── Print ───────────────────────────────────────────────────────────────────

void LL1Parser::printFirstFollowSets() const {
    std::cout << "\n========== FIRST SETS ==========\n";
    for (auto& [nt, s] : FIRST) {
        std::cout << "  FIRST(" << std::left << std::setw(18) << (nt+")") << "= { ";
        for (auto& t : s) std::cout << t << " ";
        std::cout << "}\n";
    }
    std::cout << "\n========== FOLLOW SETS ==========\n";
    for (auto& [nt, s] : FOLLOW) {
        std::cout << "  FOLLOW(" << std::left << std::setw(17) << (nt+")") << "= { ";
        for (auto& t : s) std::cout << t << " ";
        std::cout << "}\n";
    }
    std::cout << "=================================\n";
}

void LL1Parser::printParsingTable() const {
    std::cout << "\n========== LL(1) PARSING TABLE ==========\n";
    std::cout << std::left
              << std::setw(22) << "NonTerminal"
              << std::setw(18) << "Terminal"
              << "Production\n";
    std::cout << std::string(80,'-') << "\n";
    for (auto& [key, prod] : table) {
        std::string rhs;
        for (auto& s : prod) rhs += s + " ";
        if (prod.empty()) rhs = "ε";
        std::cout << std::left
                  << std::setw(22) << key.first
                  << std::setw(18) << key.second
                  << rhs << "\n";
    }
    std::cout << "=========================================\n";
}