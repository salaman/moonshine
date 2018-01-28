#pragma once

#include <moonshine/lexer/TokenType.h>

#include <string>

namespace moonshine {

struct Token
{
    Token(const TokenType& type, const std::string& value, const unsigned long& position, const unsigned long& line, const unsigned long& column)
        : type(type), value(value), position(position), line(line), column(column)
    {}

    // token
    const TokenType type;
    const std::string value;

    // positional information
    const unsigned long position;
    const unsigned long line;
    const unsigned long column;

    const char* name() const
    {
        return TokenName[type];
    }
};

}