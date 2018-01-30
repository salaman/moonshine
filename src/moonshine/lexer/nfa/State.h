#pragma once

#include "moonshine/lexer/TokenType.h"

#include <vector>
#include <utility>
#include <memory>
#include <string>

namespace moonshine { namespace nfa {

class State
{
public:
    State();
    const TokenType& getToken() const;
    void setToken(const TokenType& token_);
private:
    TokenType token_;
};

}}
