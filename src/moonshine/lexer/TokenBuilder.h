#pragma once

#include "nfa/State.h"

#include <memory>
#include <functional>

class TokenBuilder
{
public:
    static TokenBuilder define(int tokenId);

    TokenBuilder with(std::function<void(int&)> builderFunc);
private:
    int tokenId_;
    std::unique_ptr<nfa::State> startState_;
    explicit TokenBuilder(int tokenId);
};