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
    ParseError(const ParseErrorType& type_, const std::string& value_, const unsigned long& position_)
        : type(type_), value(value_), position(position_)
    {}

    const ParseErrorType type;
    const std::string value;
    const unsigned long position;
};

}
