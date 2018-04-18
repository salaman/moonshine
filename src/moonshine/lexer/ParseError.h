#pragma once

#include "moonshine/Error.h"

#include <string>
#include <ostream>

namespace moonshine {

enum class ParseErrorType {
    E_NONE = 0,

    E_INVALID_CHARACTERS,
    E_UNTERMINATED_COMMENT
};

struct ParseError
{
    ParseError(const ParseErrorType& type_, const std::string& value_, const unsigned long& position_)
        : type(type_), value(value_), position(position_)
    {}

    const ParseErrorType type;
    const std::string value;
    const unsigned long position;

    void print(std::ostream& errorOutput) const
    {
        errorOutput << "[Lexer] Error: ";

        switch (type) {
            case ParseErrorType::E_INVALID_CHARACTERS:
                errorOutput << "Invalid characters";
                break;
            case ParseErrorType::E_UNTERMINATED_COMMENT:
                errorOutput << "Unterminated comment";
                break;
            default:
                break;
        }

        errorOutput << ": \"" << value << "\" @ " << position;
    }
};

}
