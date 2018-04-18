#pragma once

#include "moonshine/Error.h"
#include "moonshine/lexer/Token.h"

#include <string>
#include <memory>
#include <algorithm>
#include <ostream>

namespace moonshine { namespace semantic {

enum class SemanticErrorType {
    UNDECLARED_VARIABLE,
    UNDECLARED_MEMBER_VARIABLE,
    UNDECLARED_MEMBER_FUNCTION,
    UNDECLARED_FUNCTION,
    UNDECLARED_CLASS,
    REDECLARED_SYMBOL,
    INCOMPATIBLE_TYPE,
    INVALID_SUBSCRIPT_TYPE,
    INVALID_DIMENSION_COUNT,
    INVALID_VARIABLE,
    INVALID_FUNCTION,
    INVALID_CLASS,
    UNDEFINED_FUNCTION,
    REDEFINED_FUNCTION,
    INCOMPATIBLE_RETURN_TYPE,
    MISSING_RETURN,
    INVALID_RETURN,
    SHADOWED_VARIABLE,
    INCORRECT_TYPE_IN_FUNCTION_DEFINITION,
    INCORRECT_TYPE_IN_FUNCTION_CALL,
    DUPLICATE_SUPER,
    CIRCULAR_INHERITANCE,
    CIRCULAR_MEMBER,
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

    SemanticErrorType type;
    std::shared_ptr<Token> token;
    SemanticErrorLevel level;

    void print(std::ostream& errorOutput) const
    {
        errorOutput << "[Semantic] ";

        switch (level) {
            case SemanticErrorLevel::ERROR:
                errorOutput << "Error: ";
                break;
            case SemanticErrorLevel::WARN:
                errorOutput << "Warning: ";
                break;
        }

        switch (type) {
            case SemanticErrorType::UNDECLARED_VARIABLE:
                errorOutput << "Undeclared variable";
                break;
            case SemanticErrorType::INCOMPATIBLE_TYPE:
                errorOutput << "Incompatible type";
                break;
            case SemanticErrorType::INVALID_SUBSCRIPT_TYPE:
                errorOutput << "Invalid subscript";
                break;
            case SemanticErrorType::INVALID_DIMENSION_COUNT:
                errorOutput << "Invalid dimension count";
                break;
            case SemanticErrorType::REDECLARED_SYMBOL:
                errorOutput << "Redeclared symbol";
                break;
            case SemanticErrorType::UNDECLARED_FUNCTION:
                errorOutput << "Undeclared function";
                break;
            case SemanticErrorType::INVALID_VARIABLE:
                errorOutput << "Invalid variable";
                break;
            case SemanticErrorType::UNDECLARED_MEMBER_VARIABLE:
                errorOutput << "Undeclared member variable";
                break;
            case SemanticErrorType::UNDECLARED_MEMBER_FUNCTION:
                errorOutput << "Undeclared member function";
                break;
            case SemanticErrorType::INVALID_FUNCTION:
                errorOutput << "Invalid function";
                break;
            case SemanticErrorType::UNDEFINED_FUNCTION:
                errorOutput << "Undefined function";
                break;
            case SemanticErrorType::REDEFINED_FUNCTION:
                errorOutput << "Redefined function";
                break;
            case SemanticErrorType::INCOMPATIBLE_RETURN_TYPE:
                errorOutput << "Incompatible return type";
                break;
            case SemanticErrorType::SHADOWED_VARIABLE:
                errorOutput << "Shadowed variable";
                break;
            case SemanticErrorType::MISSING_RETURN:
                errorOutput << "Missing return";
                break;
            case SemanticErrorType::INVALID_RETURN:
                errorOutput << "Invalid return";
                break;
            case SemanticErrorType::INCORRECT_TYPE_IN_FUNCTION_DEFINITION:
                errorOutput << "Incorrect type in function definition";
                break;
            case SemanticErrorType::INCORRECT_TYPE_IN_FUNCTION_CALL:
                errorOutput << "Incorrect type in function call";
                break;
            case SemanticErrorType::DUPLICATE_SUPER:
                errorOutput << "Duplicate super";
                break;
            case SemanticErrorType::UNDECLARED_CLASS:
                errorOutput << "Undeclared class";
                break;
            case SemanticErrorType::INVALID_CLASS:
                errorOutput << "Invalid class";
                break;
            case SemanticErrorType::CIRCULAR_INHERITANCE:
                errorOutput << "Circular inheritance";
                break;
            case SemanticErrorType::CIRCULAR_MEMBER:
                errorOutput << "Circular member";
                break;
        }

        if (token) {
            errorOutput << " for " << token->value << " (" << TokenName[token->type] << ") at position " << token->position;
        }
    }
};

}}
