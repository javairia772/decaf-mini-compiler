#include "token.h"

std::string Token::typeToString() const {
    switch(type) {
        case TokenType::CLASS:          return "CLASS";
        case TokenType::VOID:           return "VOID";
        case TokenType::INT:            return "INT";
        case TokenType::DOUBLE:         return "DOUBLE";
        case TokenType::BOOL:           return "BOOL";
        case TokenType::STRING:         return "STRING";
        case TokenType::IF:             return "IF";
        case TokenType::ELSE:           return "ELSE";
        case TokenType::WHILE:          return "WHILE";
        case TokenType::FOR:            return "FOR";
        case TokenType::RETURN:         return "RETURN";
        case TokenType::BREAK:          return "BREAK";
        case TokenType::NEW:            return "NEW";
        case TokenType::NULL_TOK:       return "NULL";
        case TokenType::THIS:           return "THIS";
        case TokenType::EXTENDS:        return "EXTENDS";
        case TokenType::IMPLEMENTS:     return "IMPLEMENTS";
        case TokenType::INTERFACE:      return "INTERFACE";
        case TokenType::STATIC:         return "STATIC";
        case TokenType::PRINT:          return "PRINT";
        case TokenType::TRUE_TOK:       return "TRUE";
        case TokenType::FALSE_TOK:      return "FALSE";
        case TokenType::INT_LITERAL:    return "INT_LITERAL";
        case TokenType::DOUBLE_LITERAL: return "DOUBLE_LITERAL";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::BOOL_LITERAL:   return "BOOL_LITERAL";
        case TokenType::IDENTIFIER:     return "IDENTIFIER";
        case TokenType::PLUS:           return "PLUS";
        case TokenType::MINUS:          return "MINUS";
        case TokenType::STAR:           return "STAR";
        case TokenType::SLASH:          return "SLASH";
        case TokenType::PERCENT:        return "PERCENT";
        case TokenType::ASSIGN:         return "ASSIGN";
        case TokenType::EQ:             return "EQ";
        case TokenType::NEQ:            return "NEQ";
        case TokenType::LT:             return "LT";
        case TokenType::LTE:            return "LTE";
        case TokenType::GT:             return "GT";
        case TokenType::GTE:            return "GTE";
        case TokenType::AND:            return "AND";
        case TokenType::OR:             return "OR";
        case TokenType::NOT:            return "NOT";
        case TokenType::LPAREN:         return "LPAREN";
        case TokenType::RPAREN:         return "RPAREN";
        case TokenType::LBRACE:         return "LBRACE";
        case TokenType::RBRACE:         return "RBRACE";
        case TokenType::LBRACKET:       return "LBRACKET";
        case TokenType::RBRACKET:       return "RBRACKET";
        case TokenType::SEMICOLON:      return "SEMICOLON";
        case TokenType::COMMA:          return "COMMA";
        case TokenType::DOT:            return "DOT";
        case TokenType::EOF_TOK:        return "EOF";
        default:                        return "UNKNOWN";
    }
}