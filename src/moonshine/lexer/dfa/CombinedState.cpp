#include "CombinedState.h"

namespace dfa {

CombinedState::CombinedState(const std::vector<nfa::State>& states_)
    : states_(states_)
{
}

}