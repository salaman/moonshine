#include "NFA.h"

#include <algorithm>
#include <forward_list>
#include <functional>
#include <iostream>

namespace moonshine { namespace nfa {

NFA::NFA()
    : states_(), start_(0), final_({0}), transitions_(), alphabet_()
{
    states_.emplace_back();
}

NFA::NFA(const Atom& atom)
    : NFA()
{
    states_.emplace_back();
    transitions_.emplace(0, std::make_pair(atom, 1));
    final_ = {1};

    alphabet_.insert(atom.characters().begin(), atom.characters().end());
}

NFA::NFA(const char& character)
    : NFA(Atom::ch(character))
{
}

NFA NFA::str(const char* string)
{
    NFA temp;

    for (int i = 0; string[i] != '\0'; ++i) {
        temp = temp & NFA(string[i]);
    }

    return temp;
}

NFA NFA::kleene()
{
    NFA temp(*this);

    // insert new starting state
    temp.states_.emplace_back();
    auto newStartIndex = temp.states_.size() - 1;

    // insert new final state
    temp.states_.emplace_back();
    auto newFinalIndex = temp.states_.size() - 1;

    // add epsilon transitions from our new start state to prior final state and new final state
    temp.transitions_.emplace(newStartIndex, std::make_pair(Atom::epsilon(), temp.start_));
    temp.transitions_.emplace(newStartIndex, std::make_pair(Atom::epsilon(), newFinalIndex));

    // add epsilon transitions from prior final states to prior start state and new final state
    for (const auto& s : temp.final_) {
        temp.transitions_.emplace(s, std::make_pair(Atom::epsilon(), temp.start_));
        temp.transitions_.emplace(s, std::make_pair(Atom::epsilon(), newFinalIndex));
    }

    // set new start and final states
    temp.start_ = newStartIndex;
    temp.final_ = {newFinalIndex};

    return temp;
}

NFA NFA::optional()
{
    NFA temp(*this);

    // insert new final state
    temp.states_.emplace_back();
    auto newFinalIndex = temp.states_.size() - 1;

    // add epsilon transitions from start state to new final state
    temp.transitions_.emplace(temp.start_, std::make_pair(Atom::epsilon(), newFinalIndex));

    // add epsilon transitions from prior final states to new final state
    for (const auto& s : temp.final_) {
        temp.transitions_.emplace(s, std::make_pair(Atom::epsilon(), newFinalIndex));
    }

    // set new start and final states
    temp.final_ = {newFinalIndex};

    return temp;
}

dfa::DFA NFA::powerset()
{
    dfa::DFA temp(states_);

    // Sdfa = {}
    std::vector<size_t> unmarkedStates;

    // add e-closure(S0) to Sdfa as the start state
    std::set<size_t> start{start_};
    std::set<size_t> startEpsilonClosure = epsilonClosure(start.cbegin(), start.cend());

    auto startState = temp.addState(startEpsilonClosure);
    unmarkedStates.emplace_back(startState);

    // mark start as final if required
    for (const auto& s: temp.getState(startState)) {
        if (isFinal(s)) {
            temp.markFinal(startState, s);
            break;
        }
    }

    // while Sdfa contains unmarked states
    while (!unmarkedStates.empty()) {
        auto Tnum = unmarkedStates.back();
        std::set<size_t> T = temp.getState(Tnum);
        unmarkedStates.pop_back();

        // for each a in alphabet
        for (const auto& a : alphabet_) {
            auto moves = move(T.cbegin(), T.cend(), a);
            auto S = epsilonClosure(moves.cbegin(), moves.cend());

            if (S.empty()) {
                continue;
            }

            // if S is not in Sdfa
            // add S to Sdfa as unmarked
            auto result = temp.getOrAddState(S);

            if (result.second) {
                unmarkedStates.emplace_back(result.first);
                for (const auto& s : temp.getState(result.first)) {
                    // TODO: optimize with isAnyFinal(set)
                    if (isFinal(s)) {
                        temp.markFinal(result.first, s);
                        break;
                    }
                }
            }

            // set MoveDFA(T, a) to S
            temp.addTransition(Tnum, result.first, a);
        }
    }

    return temp;
}

NFA NFA::operator&(const NFA& rhs) const
{
    NFA temp(*this);

    size_t reindex = temp.states_.size();

    // merge both alphabets
    temp.alphabet_.insert(rhs.alphabet_.begin(), rhs.alphabet_.end());

    // merge both state lists together
    temp.states_.reserve(temp.states_.size() + rhs.states_.size());
    temp.states_.insert(temp.states_.end(), rhs.states_.begin(), rhs.states_.end());

    // merge 2nd NFA's transitions, fixing indices
    for (const auto& p : rhs.transitions_) {
        temp.transitions_.emplace(p.first + reindex, std::make_pair(p.second.first, p.second.second + reindex));
    }

    // add epsilon transitions from 1st NFA final states to 2nd NFA start state
    for (const auto& s : temp.final_) {
        temp.transitions_.emplace(s, std::make_pair(Atom::epsilon(), rhs.start_ + reindex));
    }

    // set new final states to 2nd NFA's final states
    temp.final_ = {};
    for (const auto& s : rhs.final_) {
        temp.final_.emplace(s + reindex);
    }

    return temp;
}

NFA NFA::operator|(const NFA& rhs) const
{
    NFA temp(*this);

    size_t reindex = temp.states_.size();

    // merge both alphabets
    temp.alphabet_.insert(rhs.alphabet_.begin(), rhs.alphabet_.end());

    // merge both state lists together
    temp.states_.reserve(temp.states_.size() + rhs.states_.size());
    temp.states_.insert(temp.states_.end(), rhs.states_.begin(), rhs.states_.end());

    // merge 2nd NFA's transitions, fixing indices
    for (const auto& p : rhs.transitions_) {
        temp.transitions_.emplace(p.first + reindex, std::make_pair(p.second.first, p.second.second + reindex));
    }

    // insert new starting state
    temp.states_.emplace_back();
    auto newStartIndex = temp.states_.size() - 1;

    // add epsilon transitions from our new start state to both prior start states
    temp.transitions_.emplace(newStartIndex, std::make_pair(Atom::epsilon(), temp.start_));
    temp.transitions_.emplace(newStartIndex, std::make_pair(Atom::epsilon(), rhs.start_ + reindex));

    // set the new start state
    temp.start_ = newStartIndex;

    // merge in the 2nd NFA's final states
    for (const auto& s : rhs.final_) {
        temp.final_.emplace(s + reindex);
    }

    return temp;
}

template<typename Iter>
std::set<size_t> NFA::epsilonClosure(Iter begin, Iter end)
{
    std::vector<size_t> reachableStates(begin, end);

    size_t oldSize = 0;
    size_t size = reachableStates.size();

    while (oldSize != size) {
        for (std::vector<int>::size_type i = oldSize; i < size; ++i) {
            auto outTransitions = transitions_.equal_range(reachableStates[i]);

            for (auto t = outTransitions.first; t != outTransitions.second; ++t) {
                if (t->second.first.matches()) {
                    reachableStates.emplace_back(t->second.second);
                }
            }
        }

        oldSize = size;
        size = reachableStates.size();
    }

    std::set<size_t> ret(reachableStates.begin(), reachableStates.end());

    return ret;
}

template<typename Iter>
std::set<size_t> NFA::move(Iter begin, Iter end, const char& character)
{
    std::set<size_t> reachableStates;

    while (begin != end) {
        auto outTransitions = transitions_.equal_range(*begin);

        for (auto t = outTransitions.first; t != outTransitions.second; ++t) {
            if (t->second.first.matches(character)) {
                reachableStates.emplace(t->second.second);
            }
        }

        ++begin;
    }

    return reachableStates;
}

bool NFA::isFinal(const size_t& index) const
{
    return final_.find(index) != final_.end();
}

NFA& NFA::token(const TokenType& token)
{
    for (const auto& s : final_) {
        states_[s].setToken(token);
    }

    return *this;
}

void NFA::graphviz() const
{
    std::cout << "digraph nfa {" << std::endl;

    std::cout << R"(    forcelabels=true)" << std::endl;
    std::cout << R"(    "" [shape=none])" << std::endl;

    for (const auto& f : final_) {
        std::cout << "    \"" << f << R"(" [shape=doublecircle, label=")" << TokenName[states_[f].getToken()] << "\"]" << std::endl;
    }

    // start state
    std::cout << R"(    "" -> ")" << start_ << "\"" << std::endl;

    for (const auto& t : transitions_) {
        std::cout << "    \"" << t.first << "\" -> \"" << t.second.second << "\"";
        std::cout << " [label=\"" << t.second.first.label() ;

        //if ()

        std::cout << "\"]" << std::endl;
    }

    std::cout << "}" << std::endl;
}

}}