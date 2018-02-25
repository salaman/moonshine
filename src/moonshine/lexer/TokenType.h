#pragma once

#include <map>

namespace moonshine {

enum class TokenType : size_t
{
    T_NONE = 0,

    // type atoms
    T_IDENTIFIER,
    T_INTEGER_LITERAL,
    T_FLOAT_LITERAL,

    // comparison operators
    T_IS_EQUAL,
    T_IS_NOT_EQUAL,
    T_IS_SMALLER,
    T_IS_SMALLER_OR_EQUAL,
    T_IS_GREATER,
    T_IS_GREATER_OR_EQUAL,

    // arithmetic operators
    T_PLUS,
    T_MINUS,
    T_MUL,
    T_DIV,

    // assignment operators
    T_EQUAL,

    // logical operators
    T_AND,
    T_NOT,
    T_OR,

    // control flow keywords
    T_IF,
    T_THEN,
    T_ELSE,
    T_FOR,
    T_GET,
    T_PUT,
    T_RETURN,
    T_PROGRAM,

    // type declarations
    T_CLASS,
    T_INT,
    T_FLOAT,

    // punctuation
    T_SEMICOLON,
    T_COMMA,
    T_PERIOD,
    T_COLON,
    T_DOUBLE_COLON,
    T_OPEN_PARENTHESIS,
    T_CLOSE_PARENTHESIS,
    T_OPEN_BRACE,
    T_CLOSE_BRACE,
    T_OPEN_BRACKET,
    T_CLOSE_BRACKET,

    // comments
    //T_COMMENT,
    //T_OPEN_COMMENT,
    //T_CLOSE_COMMENT,

    TokenTypeCount
};

extern std::map<TokenType, const char*> TokenName;

}
