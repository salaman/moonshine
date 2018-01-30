#include "DFASimulator.h"

namespace moonshine { namespace dfa {

DFASimulator::DFASimulator(const DFA& dfa)
    : dfa_(dfa), currentState_(0), halted_(false)
{
}

bool DFASimulator::hasMove(const char& character)
{
    return dfa_.getTransition(currentState_, character).second;
}

void DFASimulator::move(const char& character)
{
    if (halted_) {
        return;
    }

    auto move = dfa_.getTransition(currentState_, character);

    if (!move.second) {
        halted_ = true;
        return;
    }

    currentState_ = move.first;
}

bool DFASimulator::accepted() const
{
    return !halted_ && dfa_.isFinal(currentState_);
}

bool DFASimulator::halted() const
{
    return halted_;
}

TokenType DFASimulator::token() const
{
    auto state = dfa_.getNFAStateForFinalState(currentState_);

    return state.getToken();
}

void DFASimulator::reset()
{
    currentState_ = 0;
    halted_ = false;
}

}}
