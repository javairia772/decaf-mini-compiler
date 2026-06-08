#pragma once
#include "token.h"
#include "error_handler.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <stack>

enum class LRAction { SHIFT, REDUCE, ACCEPT, ERROR };

struct Action {
    LRAction type = LRAction::ERROR;
    int      val  = 0;
};

struct Production {
    std::string              lhs;
    std::vector<std::string> rhs;
    std::string str() const {
        std::string s = lhs + " ->";
        if (rhs.empty()) return s + " ε";
        for (auto& x : rhs) s += " " + x;
        return s;
    }
};

struct Item {
    int prodIdx;
    int dot;
    bool operator<(const Item& o) const {
        return prodIdx < o.prodIdx ||
               (prodIdx == o.prodIdx && dot < o.dot);
    }
    bool operator==(const Item& o) const {
        return prodIdx == o.prodIdx && dot == o.dot;
    }
};

using ItemSet      = std::set<Item>;
using LookaheadMap = std::map<std::pair<int,Item>, std::set<std::string>>;

class LALRParser {
public:
    LALRParser(const std::vector<Token>& tokens, ErrorHandler& eh);
    bool parse();
    void printActionGotoTable() const;

private:
    std::vector<Token>      tokens;
    int                     pos;
    ErrorHandler&           errHandler;

    std::vector<Production> grammar;
    std::vector<ItemSet>    states;
    LookaheadMap            lookaheads;

    // KEY FIX: store transitions explicitly during automaton construction
    std::map<std::pair<int,std::string>, int> transitions;

    std::map<std::pair<int,std::string>, Action> ACTION;
    std::map<std::pair<int,std::string>, int>    GOTO;
    std::map<std::string, std::set<std::string>> followSets;

    void buildGrammar();

    ItemSet closure(const ItemSet& I);
    ItemSet goTo(const ItemSet& I, const std::string& X);
    void    buildCanonicalCollection();
    int     findState(const ItemSet& I);
    void    computeLookaheads();
    void    fillTable();

    std::string symOf(const Token& t) const;
    bool        isNT(const std::string& s) const;

    std::set<std::string> allNTs;
    std::map<std::string, std::set<std::string>> firstCache;
    void buildFirstCache();
    std::set<std::string> firstOf(const std::string& s);
    std::set<std::string> firstOfSeq(const std::vector<std::string>& seq,
                                      const std::set<std::string>& la);
};