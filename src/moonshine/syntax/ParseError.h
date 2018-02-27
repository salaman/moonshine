#pragma once

#include "moonshine/lexer/Token.h"

#include <string>
#include <memory>

namespace moonshine { namespace syntax {

enum class ParseErrorType {
    E_NONE = 0,

    E_UNEXPECTED_EOF,
    E_UNEXPECTED_TOKEN,

    ParseErrorTypeCount
};

struct ParseError
{
    ParseError(const ParseErrorType& type_, const std::shared_ptr<Token>& token_)
        : type(type_), token(token_)
    {}

    const ParseErrorType type;
    const std::shared_ptr<Token> token;
};

}}
