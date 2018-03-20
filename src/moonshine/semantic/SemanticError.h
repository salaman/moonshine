#pragma once

#include "moonshine/lexer/Token.h"

#include <string>
#include <memory>

namespace moonshine { namespace semantic {

enum class SemanticErrorType {
    UNDECLARED_VARIABLE,
    UNDECLARED_MEMBER_VARIABLE,
    UNDECLARED_MEMBER_FUNCTION,
    UNDECLARED_FUNCTION,
    REDECLARED_SYMBOL,
    INCOMPATIBLE_TYPE,
    INVALID_SUBSCRIPT_TYPE,
    INVALID_DIMENSION_COUNT,
    INVALID_VARIABLE,
    INVALID_FUNCTION,
    UNDEFINED_FUNCTION,
    REDEFINED_FUNCTION,
    INCOMPATIBLE_RETURN_TYPE,
    MISSING_RETURN,
    INVALID_RETURN,
    SHADOWED_VARIABLE,
};

enum class SemanticErrorLevel {
    ERROR,
    WARN,
};

struct SemanticError
{
    SemanticError(const SemanticErrorType& type_, const std::shared_ptr<Token>& token_)
        : SemanticError(type_, token_, SemanticErrorLevel::ERROR)
    {}

    SemanticError(const SemanticErrorType& type_, const std::shared_ptr<Token>& token_, const SemanticErrorLevel& level_)
        : type(type_), token(token_), level(level_)
    {}

    const SemanticErrorType type;
    const std::shared_ptr<Token> token;
    const SemanticErrorLevel level;
};

}}
