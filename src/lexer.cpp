#include "lexer.h"
#include <iostream>
#include <iomanip>

std::unordered_map<std::string, TokenType> Lexer::keywords = {
    {"class",      TokenType::CLASS},
    {"void",       TokenType::VOID},
    {"int",        TokenType::INT},
    {"double",     TokenType::DOUBLE},
    {"bool",       TokenType::BOOL},
    {"string",     TokenType::STRING},
    {"if",         TokenType::IF},
    {"else",       TokenType::ELSE},
    {"while",      TokenType::WHILE},
    {"for",        TokenType::FOR},
    {"return",     TokenType::RETURN},
    {"break",      TokenType::BREAK},
    {"new",        TokenType::NEW},
    {"null",       TokenType::NULL_TOK},
    {"this",       TokenType::THIS},
    {"extends",    TokenType::EXTENDS},
    {"implements", TokenType::IMPLEMENTS},
    {"interface",  TokenType::INTERFACE},
    {"static",     TokenType::STATIC},
    {"true",       TokenType::TRUE_TOK},
    {"false",      TokenType::FALSE_TOK},
    {"Print",      TokenType::PRINT},
};

Lexer::Lexer(const std::string& source, ErrorHandler& eh)
    : src(source), pos(0), line(1), col(1), errHandler(eh) {}

char Lexer::peek(int offset) const {
    int idx = pos + offset;
    if (idx >= (int)src.size()) return '\0';
    return src[idx];
}

char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') { line++; col = 1; }
    else col++;
    return c;
}

void Lexer::skipWhitespaceAndComments() {
    while (pos < (int)src.size()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            // single-line comment
            while (pos < (int)src.size() && peek() != '\n') advance();
        } else if (c == '/' && peek(1) == '*') {
            // multi-line comment
            advance(); advance();
            while (pos < (int)src.size()) {
                if (peek() == '*' && peek(1) == '/') {
                    advance(); advance(); break;
                }
                advance();
            }
        } else break;
    }
}

Token Lexer::makeToken(TokenType t, const std::string& lex) {
    return Token(t, lex, line, col - (int)lex.size());
}

Token Lexer::readIdentifierOrKeyword() {
    int startCol = col;
    std::string lexeme;
    while (pos < (int)src.size() && (isalnum(peek()) || peek() == '_')) {
        lexeme += advance();
    }
    auto it = keywords.find(lexeme);
    TokenType t = (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
    return Token(t, lexeme, line, startCol);
}

Token Lexer::readNumber() {
    int startCol = col;
    std::string lexeme;
    bool isDouble = false;
    while (pos < (int)src.size() && isdigit(peek())) lexeme += advance();
    if (peek() == '.' && isdigit(peek(1))) {
        isDouble = true;
        lexeme += advance();
        while (pos < (int)src.size() && isdigit(peek())) lexeme += advance();
    }
    TokenType t = isDouble ? TokenType::DOUBLE_LITERAL : TokenType::INT_LITERAL;
    return Token(t, lexeme, line, startCol);
}

Token Lexer::readString() {
    int startCol = col;
    std::string lexeme;
    advance(); // consume opening "
    while (pos < (int)src.size() && peek() != '"') {
        if (peek() == '\\') { lexeme += advance(); }
        lexeme += advance();
    }
    if (pos < (int)src.size()) advance(); // consume closing "
    else errHandler.reportError(ErrorType::LEXICAL, line, startCol, "Unterminated string literal");
    return Token(TokenType::STRING_LITERAL, lexeme, line, startCol);
}

Token Lexer::nextToken() {
    skipWhitespaceAndComments();
    if (pos >= (int)src.size())
        return Token(TokenType::EOF_TOK, "EOF", line, col);

    int startLine = line, startCol = col;
    char c = peek();

    if (isalpha(c) || c == '_') return readIdentifierOrKeyword();
    if (isdigit(c))              return readNumber();
    if (c == '"')                return readString();

    advance();
    switch (c) {
        case '+': return Token(TokenType::PLUS,      "+", startLine, startCol);
        case '-': return Token(TokenType::MINUS,     "-", startLine, startCol);
        case '*': return Token(TokenType::STAR,      "*", startLine, startCol);
        case '/': return Token(TokenType::SLASH,     "/", startLine, startCol);
        case '%': return Token(TokenType::PERCENT,   "%", startLine, startCol);
        case '(': return Token(TokenType::LPAREN,    "(", startLine, startCol);
        case ')': return Token(TokenType::RPAREN,    ")", startLine, startCol);
        case '{': return Token(TokenType::LBRACE,    "{", startLine, startCol);
        case '}': return Token(TokenType::RBRACE,    "}", startLine, startCol);
        case '[': return Token(TokenType::LBRACKET,  "[", startLine, startCol);
        case ']': return Token(TokenType::RBRACKET,  "]", startLine, startCol);
        case ';': return Token(TokenType::SEMICOLON, ";", startLine, startCol);
        case ',': return Token(TokenType::COMMA,     ",", startLine, startCol);
        case '.': return Token(TokenType::DOT,       ".", startLine, startCol);
        case '=':
            if (peek() == '=') { advance(); return Token(TokenType::EQ,  "==", startLine, startCol); }
            return Token(TokenType::ASSIGN, "=", startLine, startCol);
        case '!':
            if (peek() == '=') { advance(); return Token(TokenType::NEQ, "!=", startLine, startCol); }
            return Token(TokenType::NOT,    "!",  startLine, startCol);
        case '<':
            if (peek() == '=') { advance(); return Token(TokenType::LTE, "<=", startLine, startCol); }
            return Token(TokenType::LT,  "<", startLine, startCol);
        case '>':
            if (peek() == '=') { advance(); return Token(TokenType::GTE, ">=", startLine, startCol); }
            return Token(TokenType::GT,  ">", startLine, startCol);
        case '&':
            if (peek() == '&') { advance(); return Token(TokenType::AND, "&&", startLine, startCol); }
            break;
        case '|':
            if (peek() == '|') { advance(); return Token(TokenType::OR,  "||", startLine, startCol); }
            break;
        default: break;
    }
    errHandler.reportError(ErrorType::LEXICAL, startLine, startCol,
        std::string("Unknown character '") + c + "'");
    return Token(TokenType::UNKNOWN, std::string(1,c), startLine, startCol);
}

std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;
    while (true) {
        Token t = nextToken();
        tokens.push_back(t);
        if (t.type == TokenType::EOF_TOK) break;
    }
    return tokens;
}

void Lexer::printTokenStream(const std::vector<Token>& tokens) const {
    std::cout << "\n========== TOKEN STREAM ==========\n";
    std::cout << std::left
              << std::setw(20) << "LEXEME"
              << std::setw(20) << "TOKEN TYPE"
              << std::setw(8)  << "LINE"
              << std::setw(8)  << "COL" << "\n";
    std::cout << std::string(56, '-') << "\n";
    for (const auto& t : tokens) {
        std::cout << std::left
                  << std::setw(20) << t.lexeme
                  << std::setw(20) << t.typeToString()
                  << std::setw(8)  << t.line
                  << std::setw(8)  << t.col << "\n";
    }
    std::cout << "==================================\n";
}