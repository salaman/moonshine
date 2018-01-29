#include "TokenType.h"

#include <map>

namespace moonshine {

std::map<TokenType, const char*> TokenName = {
    {TokenType::T_NONE, "none"},
    {TokenType::T_IDENTIFIER, "identifier"},
    {TokenType::T_INTEGER_LITERAL, "integer_literal"},
    {TokenType::T_FLOAT_LITERAL, "float_literal"},
    {TokenType::T_COMMENT, "comment"},
    {TokenType::T_OPEN_COMMENT, "open_comment"},
    {TokenType::T_CLOSE_COMMENT, "close_comment"},
    {TokenType::T_IS_EQUAL, "eq"},
    {TokenType::T_IS_NOT_EQUAL, "neq"},
    {TokenType::T_IS_SMALLER, "lt"},
    {TokenType::T_IS_SMALLER_OR_EQUAL, "lte"},
    {TokenType::T_IS_GREATER, "gt"},
    {TokenType::T_IS_GREATER_OR_EQUAL, "gte"},
    {TokenType::T_PLUS, "plus"},
    {TokenType::T_MINUS, "minus"},
    {TokenType::T_MUL, "mul"},
    {TokenType::T_DIV, "div"},
    {TokenType::T_EQUAL, "equal"},
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
    {TokenType::T_DOUBLE_COLON, "double_colon"},
    {TokenType::T_OPEN_PARENTHESIS, "open_paren"},
    {TokenType::T_CLOSE_PARENTHESIS, "close_paren"},
    {TokenType::T_OPEN_BRACE, "open_brace"},
    {TokenType::T_CLOSE_BRACE, "close_brace"},
    {TokenType::T_OPEN_BRACKET, "open_bracket"},
    {TokenType::T_CLOSE_BRACKET, "close_bracket"},
};

}