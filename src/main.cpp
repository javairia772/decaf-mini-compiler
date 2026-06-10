#include <iostream>
#include <fstream>
#include <sstream>
#include "token.h"
#include "lexer.h"
#include "error_handler.h"
#include "symbol_table.h"
#include "rd_parser.h"
#include "ll1_parser.h"
#include "lr_parser.h"

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "Cannot open: " << path << "\n";
        return "";
    }
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}

int main(int argc, char* argv[]) {
    std::string src;
    if (argc >= 2) {
        src = readFile(argv[1]);
        if (src.empty()) return 1;
    } else {
        src = R"(
class Main {
    int x;
    void main(int argc) {
        int a;
        a = 5 + 3;
        if (a < 10) {
            return a;
        }
        while (a > 0) {
            a = a - 1;
        }
    }
}
)";
    }

    std::cout
        << "============================================================\n"
        << "      DECAF MINI COMPILER  |  UET Lahore  |  CS-471L\n"
        << "============================================================\n"
        << "\nSource:\n" << src << "\n";

    ErrorHandler globalErr;

    // ── LEXER ────────────────────────────────────────────────────────
    Lexer lexer(src, globalErr);
    auto tokens = lexer.tokenizeAll();
    lexer.printTokenStream(tokens);

    // ── RD PARSER ────────────────────────────────────────────────────
    {
        ErrorHandler e; SymbolTableManager st(e);
        RDParser p(tokens, st, e);
        p.parse();
        st.printAll();
        e.printSummary();
    }

    // ── LL(1) PARSER ─────────────────────────────────────────────────
    {
        ErrorHandler e;
        LL1Parser p(tokens, e);
        p.printFirstFollowSets();
        p.printParsingTable();
        p.parse();
        e.printSummary();
    }

    // ── LALR(1) PARSER ───────────────────────────────────────────────
    {
        ErrorHandler e;
        LALRParser p(tokens, e);
        p.printActionGotoTable();
        p.parse();
        e.printSummary();
    }

    globalErr.printSummary();
    return globalErr.hasErrors() ? 1 : 0;
}