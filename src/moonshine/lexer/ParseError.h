#pragma once

#include <string>

namespace moonshine {

enum class ParseErrorType {
    E_NONE = 0,

    E_INVALID_CHARACTERS,
    E_UNTERMINATED_COMMENT,

    ParseErrorTypeCount
};

struct ParseError
{
    ParseError(const ParseErrorType& type, const std::string& value, const unsigned long& position)
        : type(type), value(value), position(position)
    {}

    const ParseErrorType type;
    const std::string value;
    const unsigned long position;
};

}