#pragma once

#include <moonshine/lexer/TokenType.h>

#include <string>

namespace moonshine {

struct Token
{
    Token(const TokenType& type, const std::string& value, const unsigned int& position)
        : type(type), value(value), position(position)
    {}

    const TokenType type;
    const std::string value;
    const unsigned int position;

    const char* name() const
    {
        return TokenName[type];
    }
};

}