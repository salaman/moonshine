#include "State.h"

namespace moonshine { namespace nfa {

State::State()
    : token_(TokenType::T_NONE)
{
}

const TokenType& State::getToken() const
{
    return token_;
}

void State::setToken(const TokenType& token)
{
    token_ = token;
}

}}
