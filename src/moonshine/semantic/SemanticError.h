#pragma once

//#include "moonshine/lexer/Token.h"

#include <string>
#include <memory>

namespace moonshine { namespace semantic {

enum class SemanticErrorType {
    UNDECLARED_VARIABLE,
    REDECLARED_SYMBOL,
    INCOMPATIBLE_TYPE,
    INVALID_SUBSCRIPT_TYPE,
    INVALID_DIMENSION_COUNT,
};

enum class SemanticErrorLevel {
    ERROR,
    WARN,
};

struct SemanticError
{
    explicit SemanticError(const SemanticErrorType& type_)
        : SemanticError(type_, SemanticErrorLevel::ERROR)
    {}

    SemanticError(const SemanticErrorType& type_, const SemanticErrorLevel& level_)
        : type(type_), level(level_)
    {}

    const SemanticErrorType type;
    const SemanticErrorLevel level;
};

}}
