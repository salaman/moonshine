#pragma once

//#include "moonshine/lexer/Token.h"

#include <string>
#include <memory>

namespace moonshine { namespace semantic {

enum class SemanticErrorType {
    NONE = 0,

    UNDECLARED_VARIABLE,
    INCOMPATIBLE_TYPE,
    INVALID_SUBSCRIPT_TYPE,
    INVALID_DIMENSION_COUNT,
};

struct SemanticError
{
    explicit SemanticError(const SemanticErrorType& type_)
        : type(type_)
    {}

    const SemanticErrorType type;
    //const std::shared_ptr<Token> token;
};

}}
