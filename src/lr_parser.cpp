#include "lr_parser.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

// ─── Constructor ─────────────────────────────────────────────────────────────

LALRParser::LALRParser(const std::vector<Token>& toks, ErrorHandler& eh)
    : tokens(toks), pos(0), errHandler(eh) {
    buildGrammar();
    buildFirstCache();
    buildCanonicalCollection();  // ← was missing
    computeLookaheads();         // ← was missing
    fillTable();                 // ← was missing
    std::cout << "[SLR(1)] States: " << states.size()
              << "  ACTION: " << ACTION.size()
              << "  GOTO: "   << GOTO.size() << "\n";
}

// ─── Grammar ─────────────────────────────────────────────────────────────────

void LALRParser::buildGrammar() {
    // 0: augmented start
    grammar.push_back({"Program'",  {"Program"}});
    // 1-2: Program
    grammar.push_back({"Program",   {"ClassDecl"}});
    grammar.push_back({"Program",   {"ClassDecl","Program"}});
    // 3-4: ClassDecl
    grammar.push_back({"ClassDecl",
        {"CLASS","IDENTIFIER","LBRACE","FieldList","RBRACE"}});
    grammar.push_back({"ClassDecl",
        {"CLASS","IDENTIFIER","EXTENDS","IDENTIFIER",
         "LBRACE","FieldList","RBRACE"}});
    // 5-6: FieldList
    grammar.push_back({"FieldList",  {}});                         // ε
    grammar.push_back({"FieldList",  {"Field","FieldList"}});
    // 7-8: Field
    grammar.push_back({"Field",
        {"Type","IDENTIFIER","SEMICOLON"}});
    grammar.push_back({"Field",
        {"Type","IDENTIFIER","LPAREN","OptFormals","RPAREN","StmtBlock"}});
    // 9-14: Type
    grammar.push_back({"Type", {"INT"}});
    grammar.push_back({"Type", {"DOUBLE"}});
    grammar.push_back({"Type", {"BOOL"}});
    grammar.push_back({"Type", {"STRING"}});
    grammar.push_back({"Type", {"VOID"}});
    grammar.push_back({"Type", {"IDENTIFIER"}});
    // 15-16: OptFormals
    grammar.push_back({"OptFormals", {}});                         // ε
    grammar.push_back({"OptFormals", {"Formals"}});
    // 17-18: Formals
    grammar.push_back({"Formals", {"Type","IDENTIFIER"}});
    grammar.push_back({"Formals", {"Formals","COMMA","Type","IDENTIFIER"}});
    // 19: StmtBlock
    grammar.push_back({"StmtBlock",
        {"LBRACE","StmtList","RBRACE"}});
    // 20-21: StmtList
    grammar.push_back({"StmtList", {}});                           // ε
    grammar.push_back({"StmtList", {"Stmt","StmtList"}});
    // 22-30: Stmt
    grammar.push_back({"Stmt", {"Expr","SEMICOLON"}});
    grammar.push_back({"Stmt", {"RETURN","Expr","SEMICOLON"}});
    grammar.push_back({"Stmt", {"RETURN","SEMICOLON"}});
    grammar.push_back({"Stmt",
        {"IF","LPAREN","Expr","RPAREN","Stmt"}});
    grammar.push_back({"Stmt",
        {"IF","LPAREN","Expr","RPAREN","Stmt","ELSE","Stmt"}});
    grammar.push_back({"Stmt",
        {"WHILE","LPAREN","Expr","RPAREN","Stmt"}});
    grammar.push_back({"Stmt",
        {"FOR","LPAREN","OptExpr","SEMICOLON",
         "OptExpr","SEMICOLON","OptExpr","RPAREN","Stmt"}});
    grammar.push_back({"Stmt", {"BREAK","SEMICOLON"}});
    grammar.push_back({"Stmt", {"StmtBlock"}});
    grammar.push_back({"Stmt", {"SEMICOLON"}});

    // Local variable declarations as statements
    grammar.push_back({"Stmt", {"Type","IDENTIFIER","SEMICOLON"}});
    grammar.push_back({"Stmt", {"Type","IDENTIFIER","ASSIGN","Expr","SEMICOLON"}});

    // Print statement
    grammar.push_back({"Stmt", {"PRINT","LPAREN","Expr","MoreArgs","RPAREN","SEMICOLON"}});
    grammar.push_back({"MoreArgs", {}});
    grammar.push_back({"MoreArgs", {"COMMA","Expr","MoreArgs"}});
    // 31: OptExpr
    grammar.push_back({"OptExpr", {}});                            // ε
    grammar.push_back({"OptExpr", {"Expr"}});
    // Expressions — left recursive (fine for LR)
    // 33: Expr = Expr
    grammar.push_back({"Expr", {"Expr","ASSIGN","Expr"}});
    grammar.push_back({"Expr", {"Expr","OR","Expr"}});
    grammar.push_back({"Expr", {"Expr","AND","Expr"}});
    grammar.push_back({"Expr", {"Expr","EQ","Expr"}});
    grammar.push_back({"Expr", {"Expr","NEQ","Expr"}});
    grammar.push_back({"Expr", {"Expr","LT","Expr"}});
    grammar.push_back({"Expr", {"Expr","LTE","Expr"}});
    grammar.push_back({"Expr", {"Expr","GT","Expr"}});
    grammar.push_back({"Expr", {"Expr","GTE","Expr"}});
    grammar.push_back({"Expr", {"Expr","PLUS","Expr"}});
    grammar.push_back({"Expr", {"Expr","MINUS","Expr"}});
    grammar.push_back({"Expr", {"Expr","STAR","Expr"}});
    grammar.push_back({"Expr", {"Expr","SLASH","Expr"}});
    grammar.push_back({"Expr", {"Expr","PERCENT","Expr"}});
    grammar.push_back({"Expr", {"NOT","Expr"}});
    grammar.push_back({"Expr", {"MINUS","Expr"}});
    grammar.push_back({"Expr", {"Expr","LBRACKET","Expr","RBRACKET"}});
    grammar.push_back({"Expr", {"Expr","DOT","IDENTIFIER"}});
    grammar.push_back({"Expr",
        {"Expr","DOT","IDENTIFIER","LPAREN","Actuals","RPAREN"}});
    grammar.push_back({"Expr",
        {"IDENTIFIER","LPAREN","Actuals","RPAREN"}});
    grammar.push_back({"Expr", {"LPAREN","Expr","RPAREN"}});
    grammar.push_back({"Expr", {"THIS"}});
    grammar.push_back({"Expr",
        {"NEW","IDENTIFIER","LPAREN","RPAREN"}});
    grammar.push_back({"Expr", {"NULL"}});
    grammar.push_back({"Expr", {"TRUE"}});
    grammar.push_back({"Expr", {"FALSE"}});
    grammar.push_back({"Expr", {"INT_LITERAL"}});
    grammar.push_back({"Expr", {"DOUBLE_LITERAL"}});
    grammar.push_back({"Expr", {"STRING_LITERAL"}});
    grammar.push_back({"Expr", {"IDENTIFIER"}});
    // Actuals
    grammar.push_back({"Actuals", {}});
    grammar.push_back({"Actuals", {"Expr"}});
    grammar.push_back({"Actuals", {"Actuals","COMMA","Expr"}});

    for (auto& p : grammar) allNTs.insert(p.lhs);
}

bool LALRParser::isNT(const std::string& s) const {
    return allNTs.count(s) > 0;
}

// ─── FIRST cache ──────────────────────────────────────────────────────────────

void LALRParser::buildFirstCache() {
    for (auto& nt : allNTs) firstCache[nt] = {};
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& p : grammar) {
            auto& lhs = p.lhs;
            if (p.rhs.empty()) {
                if (!firstCache[lhs].count("EPS")) {
                    firstCache[lhs].insert("EPS"); changed = true;
                }
                continue;
            }
            bool allEps = true;
            for (auto& sym : p.rhs) {
                if (!isNT(sym)) {
                    if (!firstCache[lhs].count(sym)) {
                        firstCache[lhs].insert(sym); changed = true;
                    }
                    allEps = false; break;
                }
                for (auto& t : firstCache[sym]) {
                    if (t != "EPS" && !firstCache[lhs].count(t)) {
                        firstCache[lhs].insert(t); changed = true;
                    }
                }
                if (!firstCache[sym].count("EPS")) { allEps = false; break; }
            }
            if (allEps && !firstCache[lhs].count("EPS")) {
                firstCache[lhs].insert("EPS"); changed = true;
            }
        }
    }
}

std::set<std::string> LALRParser::firstOf(const std::string& s) {
    if (!isNT(s)) return {s};
    return firstCache.count(s) ? firstCache[s] : std::set<std::string>{};
}

std::set<std::string> LALRParser::firstOfSeq(
        const std::vector<std::string>& seq,
        const std::set<std::string>& la) {
    std::set<std::string> res;
    bool allEps = true;
    for (auto& sym : seq) {
        auto f = firstOf(sym);
        for (auto& t : f) if (t != "EPS") res.insert(t);
        if (!f.count("EPS")) { allEps = false; break; }
    }
    if (allEps) for (auto& t : la) res.insert(t);
    return res;
}

// ─── LR(0) Closure & Goto ─────────────────────────────────────────────────────

ItemSet LALRParser::closure(const ItemSet& I) {
    ItemSet result = I;
    bool changed = true;
    while (changed) {
        changed = false;
        // copy to avoid iterator invalidation
        ItemSet current = result;
        for (auto& item : current) {
            auto& rhs = grammar[item.prodIdx].rhs;
            if (item.dot >= (int)rhs.size()) continue;
            std::string B = rhs[item.dot];
            if (!isNT(B)) continue;
            for (int i = 0; i < (int)grammar.size(); i++) {
                if (grammar[i].lhs != B) continue;
                Item ni{i, 0};
                if (!result.count(ni)) {
                    result.insert(ni);
                    changed = true;
                }
            }
        }
    }
    return result;
}

ItemSet LALRParser::goTo(const ItemSet& I, const std::string& X) {
    ItemSet J;
    for (auto& item : I) {
        auto& rhs = grammar[item.prodIdx].rhs;
        if (item.dot < (int)rhs.size() && rhs[item.dot] == X)
            J.insert({item.prodIdx, item.dot + 1});
    }
    if (J.empty()) return {};
    return closure(J);
}

void LALRParser::buildCanonicalCollection() {
    ItemSet s0 = closure({{0, 0}});
    states.push_back(s0);

    // Also store transitions: stateIdx -> symbol -> targetStateIdx
    // We store these in a flat map for use during table construction
    transitions.clear();

    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < (int)states.size(); i++) {
            // Collect all symbols with dot before them
            std::set<std::string> syms;
            for (auto& item : states[i]) {
                auto& rhs = grammar[item.prodIdx].rhs;
                if (item.dot < (int)rhs.size())
                    syms.insert(rhs[item.dot]);
            }
            for (auto& X : syms) {
                ItemSet nxt = goTo(states[i], X);
                if (nxt.empty()) continue;
                // find or add target state
                int ti = -1;
                for (int j = 0; j < (int)states.size(); j++) {
                    if (states[j] == nxt) { ti = j; break; }
                }
                if (ti == -1) {
                    ti = (int)states.size();
                    states.push_back(nxt);
                    changed = true;
                }
                // record transition
                transitions[{i, X}] = ti;
            }
        }
    }
}

// ─── LALR lookahead propagation ───────────────────────────────────────────────

// Find the state index that contains a given item set
int LALRParser::findState(const ItemSet& I) {
    for (int i = 0; i < (int)states.size(); i++)
        if (states[i] == I) return i;
    return -1;
}

void LALRParser::computeLookaheads() {
    // We use SLR(1) lookaheads: FOLLOW(A) for every [A -> α .]
    // This is correct for Decaf's grammar and avoids the
    // complexity of full LALR lookahead propagation.
    // Build FOLLOW sets for all non-terminals.

    for (auto& nt : allNTs) followSets[nt] = {};
    followSets["Program'"].insert("$");

    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& prod : grammar) {
            for (int i = 0; i < (int)prod.rhs.size(); i++) {
                std::string B = prod.rhs[i];
                if (!isNT(B)) continue;

                // Add FIRST(β) - {ε} to FOLLOW(B)
                std::vector<std::string> beta(
                    prod.rhs.begin() + i + 1, prod.rhs.end());

                bool betaHasEps = true;
                for (auto& sym : beta) {
                    if (!isNT(sym)) {
                        if (!followSets[B].count(sym)) {
                            followSets[B].insert(sym);
                            changed = true;
                        }
                        betaHasEps = false;
                        break;
                    }
                    for (auto& t : firstCache[sym]) {
                        if (t != "EPS" && !followSets[B].count(t)) {
                            followSets[B].insert(t);
                            changed = true;
                        }
                    }
                    if (!firstCache[sym].count("EPS")) {
                        betaHasEps = false;
                        break;
                    }
                }

                // If β =>* ε (or β is empty), add FOLLOW(lhs) to FOLLOW(B)
                if (betaHasEps || beta.empty()) {
                    for (auto& t : followSets[prod.lhs]) {
                        if (!followSets[B].count(t)) {
                            followSets[B].insert(t);
                            changed = true;
                        }
                    }
                }
            }
        }
    }
}

void LALRParser::fillTable() {
    // SHIFT and GOTO from recorded transitions
    for (auto& [key, ti] : transitions) {
        int si        = key.first;
        std::string X = key.second;
        if (isNT(X))
            GOTO[{si, X}] = ti;
        else
            ACTION[{si, X}] = {LRAction::SHIFT, ti};
    }

    // REDUCE and ACCEPT using SLR(1) lookaheads = FOLLOW sets
    for (int si = 0; si < (int)states.size(); si++) {
        // Use full closure so epsilon-completed items are included
        ItemSet fullClosure = closure(states[si]);

        for (auto& item : fullClosure) {
            auto& rhs = grammar[item.prodIdx].rhs;
            if (item.dot < (int)rhs.size()) continue; // not complete

            std::string lhs = grammar[item.prodIdx].lhs;

            if (item.prodIdx == 0) {
                // Program' -> Program .  => ACCEPT on $
                ACTION[{si, "$"}] = {LRAction::ACCEPT, 0};
                continue;
            }

            // Reduce on every token in FOLLOW(lhs)
            for (auto& a : followSets[lhs]) {
                auto akey = std::make_pair(si, a);
                if (!ACTION.count(akey)) {
                    ACTION[akey] = {LRAction::REDUCE, item.prodIdx};
                } else if (ACTION[akey].type == LRAction::SHIFT) {
                    // Shift-reduce conflict: keep shift (standard resolution)
                } else if (ACTION[akey].type == LRAction::REDUCE) {
                    // Reduce-reduce: keep lower production number
                    if (item.prodIdx < ACTION[akey].val)
                        ACTION[akey] = {LRAction::REDUCE, item.prodIdx};
                }
            }
        }
    }
}

// ─── Symbol helper ────────────────────────────────────────────────────────────

std::string LALRParser::symOf(const Token& t) const {
    switch (t.type) {
        case TokenType::EOF_TOK:        return "$";
        case TokenType::IDENTIFIER:     return "IDENTIFIER";
        case TokenType::INT_LITERAL:    return "INT_LITERAL";
        case TokenType::DOUBLE_LITERAL: return "DOUBLE_LITERAL";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::TRUE_TOK:       return "TRUE";
        case TokenType::FALSE_TOK:      return "FALSE";
        case TokenType::NULL_TOK:       return "NULL";
        default:                        return t.typeToString();
    }
}

// ─── Parse ────────────────────────────────────────────────────────────────────

bool LALRParser::parse() {
    std::cout << "\n[LALR(1) Parser] Starting parse...\n";
    std::cout << std::string(105, '-') << "\n";
    std::cout << std::left
              << std::setw(20) << "STATE STACK"
              << std::setw(28) << "SYMBOL STACK"
              << std::setw(22) << "INPUT"
              << "ACTION\n";
    std::cout << std::string(105, '-') << "\n";

    std::stack<int>         stateStk;
    std::stack<std::string> symStk;
    stateStk.push(0);
    symStk.push("$");

    auto stkStr = [&]() {
        std::stack<int> tmp = stateStk;
        std::vector<int> v;
        while (!tmp.empty()) { v.push_back(tmp.top()); tmp.pop(); }
        std::string s;
        for (auto it = v.rbegin(); it != v.rend(); ++it)
            s += std::to_string(*it) + " ";
        return s;
    };
    auto symStr = [&]() {
        std::stack<std::string> tmp = symStk;
        std::vector<std::string> v;
        while (!tmp.empty()) { v.push_back(tmp.top()); tmp.pop(); }
        std::string s;
        for (auto it = v.rbegin(); it != v.rend(); ++it)
            s += *it + " ";
        return s;
    };
    auto inpStr = [&]() {
        std::string s;
        for (int i = pos; i < std::min((int)tokens.size(), pos+3); i++)
            s += tokens[i].lexeme + " ";
        return s;
    };

    bool success = true;
    int  limit   = 5000;

    while (limit-- > 0) {
        int         st  = stateStk.top();
        std::string sym = symOf(tokens[pos]);
        auto        key = std::make_pair(st, sym);

        if (!ACTION.count(key)) {
            errHandler.reportError(ErrorType::SYNTACTIC,
                tokens[pos].line, tokens[pos].col,
                "LALR: No action in state " +
                std::to_string(st) + " on '" + sym + "'");
            std::cout << std::left
                      << std::setw(20) << stkStr()
                      << std::setw(28) << symStr()
                      << std::setw(22) << inpStr()
                      << "ERROR — skip token\n";
            success = false;
            if (tokens[pos].type == TokenType::EOF_TOK) break;
            pos++;
            continue;
        }

        Action act = ACTION.at(key);

        if (act.type == LRAction::ACCEPT) {
            std::cout << std::left
                      << std::setw(20) << stkStr()
                      << std::setw(28) << symStr()
                      << std::setw(22) << inpStr()
                      << "ACCEPT\n";
            break;

        } else if (act.type == LRAction::SHIFT) {
            std::cout << std::left
                      << std::setw(20) << stkStr()
                      << std::setw(28) << symStr()
                      << std::setw(22) << inpStr()
                      << "SHIFT " + std::to_string(act.val) + "\n";
            stateStk.push(act.val);
            symStk.push(sym);
            pos++;

        } else { // REDUCE
            auto& prod = grammar[act.val];
            std::string actStr = "REDUCE by " + prod.str();
            std::cout << std::left
                      << std::setw(20) << stkStr()
                      << std::setw(28) << symStr()
                      << std::setw(22) << inpStr()
                      << actStr << "\n";

            for (int i = 0; i < (int)prod.rhs.size(); i++) {
                if (stateStk.size() > 1) stateStk.pop();
                if (symStk.size() > 1)   symStk.pop();
            }

            int top = stateStk.top();
            auto gkey = std::make_pair(top, prod.lhs);
            if (!GOTO.count(gkey)) {
                errHandler.reportError(ErrorType::SYNTACTIC,
                    tokens[pos].line, tokens[pos].col,
                    "LALR: GOTO missing for (" +
                    std::to_string(top) + ", " + prod.lhs + ")");
                success = false;
                break;
            }
            stateStk.push(GOTO.at(gkey));
            symStk.push(prod.lhs);
        }
    }

    std::cout << "[LALR(1) Parser] "
              << (success ? "SUCCESS" : "FAILED") << "\n";
    return success;
}

// ─── Print tables ─────────────────────────────────────────────────────────────

void LALRParser::printActionGotoTable() const {
    std::cout << "\n========== LALR(1) ACTION TABLE ==========\n";
    std::cout << std::left
              << std::setw(8)  << "STATE"
              << std::setw(20) << "TERMINAL"
              << "ACTION\n";
    std::cout << std::string(55, '-') << "\n";
    for (auto& [k, a] : ACTION) {
        std::string s;
        if      (a.type == LRAction::SHIFT)  s = "s" + std::to_string(a.val);
        else if (a.type == LRAction::REDUCE)  s = "r" + std::to_string(a.val)
            + "  (" + grammar[a.val].str() + ")";
        else if (a.type == LRAction::ACCEPT)  s = "acc";
        std::cout << std::left
                  << std::setw(8)  << k.first
                  << std::setw(20) << k.second
                  << s << "\n";
    }
    std::cout << "\n========== LALR(1) GOTO TABLE ==========\n";
    std::cout << std::left
              << std::setw(8)  << "STATE"
              << std::setw(20) << "NONTERMINAL"
              << "GOTO\n";
    std::cout << std::string(40, '-') << "\n";
    for (auto& [k, v] : GOTO)
        std::cout << std::left
                  << std::setw(8)  << k.first
                  << std::setw(20) << k.second
                  << v << "\n";
    std::cout << "=========================================\n";
}