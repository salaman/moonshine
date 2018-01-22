#pragma once

//#include "moonshine/lexer/nfa/NFA.h"
#include "moonshine/lexer/dfa/CombinedState.h"

#include <memory>
#include <set>
#include <map>
#include <utility>

namespace dfa {

typedef std::vector<nfa::State>::size_type size_t;

class DFA
{
public:
    explicit DFA(const std::vector<nfa::State>& states);
    size_t addState(const std::set<size_t>& state);
    std::pair<size_t, bool> getOrAddState(const std::set<size_t>& state);
    const std::set<size_t>& getState(const size_t& index) const;
    bool hasState(const std::set<size_t>& state) const;
    void markFinal(const size_t& index);
    bool isFinal(const size_t& index) const;
    void addTransition(const size_t& from, const size_t& to, const char& character);
    std::pair<size_t, bool> getTransition(const size_t& from, const char& character) const;
private:
    std::vector<nfa::State> nfaStates_;
    std::vector<std::set<size_t>> states_;
    size_t start_;
    std::vector<size_t> final_;
    std::map<std::pair<size_t, char>, size_t> transitions_;
};

}