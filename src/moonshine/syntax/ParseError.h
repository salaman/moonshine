#pragma once

#include "moonshine/Error.h"
#include "moonshine/lexer/Token.h"

#include <string>
#include <memory>
#include <ostream>

namespace moonshine { namespace syntax {

enum class ParseErrorType {
    E_NONE = 0,

    E_UNEXPECTED_EOF,
    E_UNEXPECTED_TOKEN
};

struct ParseError
{
    ParseError(const ParseErrorType& type_, const std::shared_ptr<Token>& token_)
        : type(type_), token(token_)
    {}

    const ParseErrorType type;
    const std::shared_ptr<Token> token;

    void print(std::ostream& errorOutput) const
    {
        errorOutput << "[Parser] ";

        switch (type) {
            case syntax::ParseErrorType::E_UNEXPECTED_TOKEN:
                errorOutput << "Error: Unexpected token \"" << token->value << "\" at position " << token->position;
                break;
            case syntax::ParseErrorType::E_UNEXPECTED_EOF:
                errorOutput << "Error: Unexpected end of file reached";
                break;
            default:
                break;
        }
    }
};

}}
