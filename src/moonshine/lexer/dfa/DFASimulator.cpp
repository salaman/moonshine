#include "DFASimulator.h"

namespace dfa {

DFASimulator::DFASimulator(const DFA& dfa)
    : dfa_(dfa), currentState_(0), halted_(false)
{
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

}