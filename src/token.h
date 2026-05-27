#pragma once
#include <string>

enum class TokenType {
    // Keywords
    CLASS, VOID, INT, DOUBLE, BOOL, STRING,
    IF, ELSE, WHILE, FOR, RETURN, BREAK,
    NEW, NULL_TOK, THIS, EXTENDS, IMPLEMENTS,
    INTERFACE, STATIC, PRIVATE, PUBLIC, PROTECTED,
    TRUE_TOK, FALSE_TOK, PRINT,

    // Literals
    INT_LITERAL, DOUBLE_LITERAL, STRING_LITERAL, BOOL_LITERAL,

    // Identifiers
    IDENTIFIER,

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT,
    ASSIGN,
    EQ, NEQ, LT, LTE, GT, GTE,
    AND, OR, NOT,

    // Delimiters
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    LBRACKET, RBRACKET,
    SEMICOLON, COMMA, DOT,

    // Special
    EOF_TOK, UNKNOWN
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int col;

    Token(TokenType t, std::string l, int ln, int c)
        : type(t), lexeme(std::move(l)), line(ln), col(c) {}

    std::string typeToString() const;
};