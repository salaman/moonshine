#include "moonshine/semantic/Type.h"

#include <sstream>

namespace moonshine { namespace semantic {

bool VariableType::compare(const SymbolType& rhs) const
{
    if (const VariableType* p = dynamic_cast<const VariableType*>(&rhs)) {
        return type == p->type;
    }

    return false;
}

std::string VariableType::str() const
{
    switch (type) {
        case Type::INT:
            return "int";
        case Type::FLOAT:
            return "float";
        case Type::CLASS:
            return className;
    }

    return "";
}

bool FunctionType::compare(const SymbolType& rhs) const
{
    if (const FunctionType* p = dynamic_cast<const FunctionType*>(&rhs)) {
        return returnType == p->returnType; // TODO: params
    }

    return false;
}

std::string FunctionType::str() const
{
    std::stringstream s;

    s << returnType.str() <<  " : ";

    for (const auto& p : parameterTypes) {
        s << p.str() << ", ";
    }

    return s.str();
}

}}