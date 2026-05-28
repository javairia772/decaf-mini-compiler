#pragma once
#include "token.h"
#include "error_handler.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    Lexer(const std::string& source, ErrorHandler& eh);
    Token nextToken();
    std::vector<Token> tokenizeAll();
    void printTokenStream(const std::vector<Token>& tokens) const;

private:
    std::string src;
    int pos, line, col;
    ErrorHandler& errHandler;
    static std::unordered_map<std::string, TokenType> keywords;

    char peek(int offset = 0) const;
    char advance();
    void skipWhitespaceAndComments();
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readString();
    Token makeToken(TokenType t, const std::string& lex);
};