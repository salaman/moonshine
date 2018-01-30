#pragma once

#include <moonshine/lexer/TokenType.h>

#include <string>

namespace moonshine {

struct Token
{
    Token(const TokenType& type_, const std::string& value_, const unsigned long& position_)
        : type(type_), value(value_), position(position_)
    {}

    const TokenType type;
    const std::string value;
    const unsigned long position;

    const char* name() const
    {
        return TokenName[type];
    }
};

}
