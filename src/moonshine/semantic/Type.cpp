#include "moonshine/semantic/Type.h"

#include <sstream>

namespace moonshine { namespace semantic {

bool VariableType::compare(const SymbolType& rhs) const
{
    if (auto p = dynamic_cast<const VariableType*>(&rhs)) {
        return type == p->type && indices == p->indices;
    }

    return false;
}

std::string VariableType::str() const
{
    std::stringstream s;

    switch (type) {
        case Type::INT:
            s << "int";
            break;
        case Type::FLOAT:
            s << "float";
            break;
        case Type::CLASS:
            s << className;
            break;
        case Type::ERROR:
            s << "ERR";
            break;
    }

    for (const auto& i : indices) {
        s << '[' << i << ']';
    }

    return s.str();
}

bool FunctionType::compare(const SymbolType& rhs) const
{
    if (auto p = dynamic_cast<const FunctionType*>(&rhs)) {
        return returnType == p->returnType && parameterTypes == p->parameterTypes;
    }

    return false;
}

std::string FunctionType::str() const
{
    std::stringstream s;

    s << returnType.str() <<  " : ";

    for (auto it = parameterTypes.begin(); it != parameterTypes.end(); ++it) {
        s << it->str();
        if (it + 1 != parameterTypes.end()) {
            s << ", ";
        }
    }

    if (parameterTypes.empty()) {
        s << "nil";
    }

    return s.str();
}

}}