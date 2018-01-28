#include "TokenType.h"

#include <map>

namespace moonshine {

std::map<TokenType, const char*> TokenName = {
    {TokenType::T_NONE, "none"},
    {TokenType::T_IDENTIFIER, "identifier"},
    {TokenType::T_INTEGER_LITERAL, "integer_literal"},
    {TokenType::T_FLOAT_LITERAL, "float_literal"},
    {TokenType::T_COMMENT, "comment"},
    {TokenType::T_IS_EQUAL, "none"},
    {TokenType::T_IS_NOT_EQUAL, "none"},
    {TokenType::T_IS_SMALLER, "none"},
    {TokenType::T_IS_SMALLER_OR_EQUAL, "none"},
    {TokenType::T_IS_GREATER, "none"},
    {TokenType::T_IS_GREATER_OR_EQUAL, "none"},
    {TokenType::T_PLUS, "none"},
    {TokenType::T_MINUS, "none"},
    {TokenType::T_MUL, "none"},
    {TokenType::T_DIV, "none"},
    {TokenType::T_EQUAL, "none"},
    {TokenType::T_AND, "and"},
    {TokenType::T_NOT, "not"},
    {TokenType::T_OR, "or"},
    {TokenType::T_IF, "if"},
    {TokenType::T_THEN, "then"},
    {TokenType::T_ELSE, "else"},
    {TokenType::T_FOR, "for"},
    {TokenType::T_GET, "get"},
    {TokenType::T_PUT, "none"},
    {TokenType::T_RETURN, "return"},
    {TokenType::T_PROGRAM, "program"},
    {TokenType::T_CLASS, "class"},
    {TokenType::T_INT, "int"},
    {TokenType::T_FLOAT, "float"},
    {TokenType::T_SEMICOLON, "semicolon"},
    {TokenType::T_COMMA, "comma"},
    {TokenType::T_PERIOD, "period"},
    {TokenType::T_COLON, "colon"},
    {TokenType::T_DOUBLE_COLON, "none"},
    {TokenType::T_OPEN_PARENTHESIS, "none"},
    {TokenType::T_CLOSE_PARENTHESIS, "none"},
    {TokenType::T_OPEN_BRACE, "none"},
    {TokenType::T_CLOSE_BRACE, "none"},
    {TokenType::T_OPEN_BRACKET, "none"},
    {TokenType::T_CLOSE_BRACKET, "none"},
};

}