#pragma once

#include <string>

namespace moonshine {

struct ParseError
{
    ParseError(const std::string& value, const unsigned long& position)
        : value(value), position(position)
    {}

    const std::string value;
    const unsigned long position;
};

}