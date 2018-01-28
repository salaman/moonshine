#include "DFA.h"

#include <exception>
#include <algorithm>
#include <iostream>

namespace moonshine { namespace dfa {

DFA::DFA(const std::vector<nfa::State>& states)
    : nfaStates_(states), states_(), start_(0), final_()
{
}

void DFA::addTransition(const size_t& from, const size_t& to, const char& character)
{
    auto result = transitions_.emplace(std::make_pair(from, character), to);

    if (!result.second) {
        //std::cout << "Error adding (" << from << ", " << character << ") -> " << to << std::endl;
        throw std::runtime_error("Duplicate transition in DFA");
    }

    //std::cout << "Added (" << from << ", " << character << ") -> " << to << " {";
    //for (const auto& i : states_[to]) {
    //    std::cout << i << ", ";
    //}
    //std::cout << "}" << std::endl;
}

size_t DFA::addState(const std::set<size_t>& state)
{
    states_.push_back(state);
    //std::cout << "Added state " << states_.size() - 1 << " {";
    //for (const auto& i : state) {
    //    std::cout << i << ", ";
    //}
    //std::cout << "}" << std::endl;
    return states_.size() - 1;
}

bool DFA::hasState(const std::set<size_t>& state) const
{
    return std::find(states_.cbegin(), states_.cend(), state) != states_.cend();
}

const std::set<size_t>& DFA::getState(const size_t& index) const
{
    return states_[index];
}

std::pair<size_t, bool> DFA::getOrAddState(const std::set<size_t>& state)
{
    size_t i;

    for (i = 0; i < states_.size(); ++i) {
        if (states_[i] == state) {
            return std::make_pair(i, false);
        }
    }

    return std::make_pair(addState(state), true);
}

void DFA::markFinal(const size_t& index, const size_t& nfaIndex)
{
    //std::cout << "Marked " << index << " as final" << std::endl;
    final_.emplace(index, nfaIndex);
}

std::pair<size_t, bool> DFA::getTransition(const size_t& from, const char& character) const
{
    auto it = transitions_.find(std::make_pair(from, character));

    if (it == transitions_.end()) {
        return std::make_pair(0, false);
    }

    return std::make_pair(it->second, true);
}

bool DFA::isFinal(const size_t& index) const
{
    return final_.find(index) != final_.end();
}

const nfa::State& DFA::getNFAState(const size_t& index) const
{
    return nfaStates_[index];
}

const nfa::State& DFA::getNFAStateForFinalState(const size_t& index) const
{
    return nfaStates_[final_.find(index)->second];
}

}}