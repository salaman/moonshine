#pragma once

#include "moonshine/lexer/nfa/State.h"
#include "moonshine/lexer/nfa/Atom.h"

#include <vector>

namespace dfa {

class CombinedState
{
public:
    explicit CombinedState(const std::vector<nfa::State>& states_);
private:
    std::vector<nfa::State> states_;
};

}