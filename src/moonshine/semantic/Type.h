#pragma once

#include <string>
#include <vector>

namespace moonshine { namespace semantic {

enum class Type
{
    INT,
    FLOAT,
    CLASS
};

struct SymbolType
{
    virtual std::string str() const = 0;

    friend bool operator==(const SymbolType& lhs, const SymbolType& rhs) {
        return lhs.compare(rhs);
    }

protected:
    virtual bool compare(const SymbolType& rhs) const = 0;
};

struct VariableType : public SymbolType
{
    Type type;
    std::string className;
    std::vector<unsigned int> indices;

    std::string str() const override;
protected:
    bool compare(const SymbolType& rhs) const override;
};

struct FunctionType : public SymbolType
{
    VariableType returnType;
    std::vector<VariableType> parameterTypes;

    std::string str() const override;
protected:
    bool compare(const SymbolType& rhs) const override;
};

}}
