#include "TokenType.h"

#include <nlohmann/json.hpp>
#include <map>

namespace moonshine {

std::map<TokenType, const char*> TokenName = {
    {TokenType::T_NONE, "none"},
    {TokenType::T_IDENTIFIER, "id"},
    {TokenType::T_INTEGER_LITERAL, "intNum"},
    {TokenType::T_FLOAT_LITERAL, "floatNum"},
    //{TokenType::T_COMMENT, "comment"},
    //{TokenType::T_OPEN_COMMENT, "open_comment"},
    //{TokenType::T_CLOSE_COMMENT, "close_comment"},
    {TokenType::T_IS_EQUAL, "eq"},
    {TokenType::T_IS_NOT_EQUAL, "neq"},
    {TokenType::T_IS_SMALLER, "lt"},
    {TokenType::T_IS_SMALLER_OR_EQUAL, "leq"},
    {TokenType::T_IS_GREATER, "gt"},
    {TokenType::T_IS_GREATER_OR_EQUAL, "geq"},
    {TokenType::T_PLUS, "+"},
    {TokenType::T_MINUS, "-"},
    {TokenType::T_MUL, "*"},
    {TokenType::T_DIV, "/"},
    {TokenType::T_EQUAL, "="},
    {TokenType::T_AND, "and"},
    {TokenType::T_NOT, "not"},
    {TokenType::T_OR, "or"},
    {TokenType::T_IF, "if"},
    {TokenType::T_THEN, "then"},
    {TokenType::T_ELSE, "else"},
    {TokenType::T_FOR, "for"},
    {TokenType::T_GET, "get"},
    {TokenType::T_PUT, "put"},
    {TokenType::T_RETURN, "return"},
    {TokenType::T_PROGRAM, "program"},
    {TokenType::T_CLASS, "class"},
    {TokenType::T_INT, "int"},
    {TokenType::T_FLOAT, "float"},
    {TokenType::T_SEMICOLON, ";"},
    {TokenType::T_COMMA, ","},
    {TokenType::T_PERIOD, "."},
    {TokenType::T_COLON, ":"},
    {TokenType::T_DOUBLE_COLON, "sr"},
    {TokenType::T_OPEN_PARENTHESIS, "("},
    {TokenType::T_CLOSE_PARENTHESIS, ")"},
    {TokenType::T_OPEN_BRACE, "{"},
    {TokenType::T_CLOSE_BRACE, "}"},
    {TokenType::T_OPEN_BRACKET, "["},
    {TokenType::T_CLOSE_BRACKET, "]"},
};

}
