#pragma once
#include "token.h"
#include "error_handler.h"
#include <vector>
#include <map>
#include <set>
#include <string>

class LL1Parser {
public:
    LL1Parser(const std::vector<Token>& tokens, ErrorHandler& eh);
    bool parse();
    void printFirstFollowSets() const;
    void printParsingTable() const;

private:
    std::vector<Token> tokens;
    int pos;
    ErrorHandler& errHandler;

    // Grammar: map from NT -> list of productions (each = vector of symbols)
    std::map<std::string, std::vector<std::vector<std::string>>> grammar;

    // FIRST and FOLLOW
    std::map<std::string, std::set<std::string>> FIRST;
    std::map<std::string, std::set<std::string>> FOLLOW;

    // LL(1) table: (NT, terminal) -> production
    std::map<std::pair<std::string,std::string>,
             std::vector<std::string>> table;

    // Terminals set
    std::set<std::string> terminals;
    std::set<std::string> nonTerminals;

    void buildGrammar();
    void computeFirstSets();
    void computeFollowSets();
    void buildTable();

    std::set<std::string> firstOfSequence(
        const std::vector<std::string>& seq) const;
    bool isNonTerminal(const std::string& s) const;
    bool isTerminal(const std::string& s) const;

    std::string tokenToTerminal(const Token& t) const;
    Token& current();
    Token  consume();
    bool   check(TokenType t) const;
};