#include "NFA.h"

#include <algorithm>
#include <forward_list>
#include <functional>

namespace nfa {

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

NFA NFA::kleene()
{
    NFA temp(*this);

    // TODO

    return temp;
}

dfa::DFA NFA::powerset()
{
    dfa::DFA temp(states_);

    // Sdfa = {}
    std::vector<size_t> unmarkedStates;

    // add e-closure(S0) to Sdfa as the start state
    std::set<size_t> start {start_};
    std::set<size_t> startEpsilonClosure = epsilonClosure(start.cbegin(), start.cend());

    unmarkedStates.emplace_back(temp.addState(startEpsilonClosure));

    // while Sdfa contains unmarked states
    while (!unmarkedStates.empty()) {
        auto Tnum = unmarkedStates.back();
        std::set<size_t> T = temp.getState(Tnum);
        unmarkedStates.pop_back();

        // for each a in alphabet
        for (const auto& a : alphabet_) {
            auto moves = move(T.cbegin(), T.cend(), a);
            auto S = epsilonClosure(moves.cbegin(), moves.cend());

            // if S is not in Sdfa
            // add S to Sdfa as unmarked
            auto result = temp.getOrAddState(S);

            if (result.second) {
                unmarkedStates.emplace_back(result.first);
                for (const auto& s : temp.getState(result.first)) {
                    // TODO: optimize with isAnyFinal(set)
                    if (isFinal(s)) {
                        temp.markFinal(result.first);
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

//dfa::DFA NFA::powerset()
//{
//    dfa::DFA temp(states_);
//
//    // Sdfa = {}
//    std::forward_list<std::set<size_t>> states;
//    std::vector<std::reference_wrapper<std::set<size_t>>> unmarkedStates;
//
//    // add e-closure(S0) to Sdfa as the start state
//    std::set<size_t> start {start_};
//    std::set<size_t> startEpsilonClosure = epsilonClosure(start.cbegin(), start.cend());
//
//    states.push_front(startEpsilonClosure);
//    unmarkedStates.emplace_back(*states.begin());
//
//    // while Sdfa contains unmarked states
//    while (!unmarkedStates.empty()) {
//        std::set<size_t> T = unmarkedStates.back();
//        unmarkedStates.pop_back();
//
//        // for each a in alphabet
//        for (const auto& a : alphabet_) {
//            auto moves = move(T.cbegin(), T.cend(), a);
//            auto S = epsilonClosure(moves.cbegin(), moves.cend());
//
//            // if S is not in Sdfa
//            if (std::find(states.cbegin(), states.cend(), S) != states.cend()) {
//                // add S to Sdfa as unmarked
//                states.push_front(S);
//                unmarkedStates.emplace_back(*states.begin());
//            }
//
//            // set MoveDFA(T, a) to S
//            temp.addTransition(T, S, a);
//        }
//    }
//
//    return temp;
//}

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

    if (temp.final_.size() != 1 || rhs.final_.size() != 1) {
        // TODO
        throw std::runtime_error("TODO NFA with mult final states");
    }

    // add epsilon transition from 1st NFA final state to 2nd NFA start state
    temp.transitions_.emplace(*temp.final_.begin(), std::make_pair(Atom::epsilon(), rhs.start_ + reindex));

    // set new final state to 2nd NFA's final state
    temp.final_ = {*rhs.final_.begin() + reindex};

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

    // add the 2nd NFA's final state
    temp.final_.emplace(*rhs.final_.cbegin() + reindex);

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
std::vector<size_t> NFA::move(Iter begin, Iter end, const char& character)
{
    std::vector<size_t> reachableStates;

    while (begin != end) {
        auto outTransitions = transitions_.equal_range(*begin);

        for (auto t = outTransitions.first; t != outTransitions.second; ++t) {
            if (t->second.first.matches(character)) {
                reachableStates.push_back(t->second.second);
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

}