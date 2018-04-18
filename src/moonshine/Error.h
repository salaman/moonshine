#pragma once

#include "moonshine/lexer/Token.h"

#include <string>
#include <memory>

namespace moonshine {

struct Error
{
    Error(const unsigned int& position_, const std::string& message_)
        : position(position_), message(message_)
    {}

    unsigned int position;
    std::string message;
};

}
