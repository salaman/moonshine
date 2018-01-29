#pragma once

#include "moonshine/lexer/nfa/State.h"
#include "moonshine/lexer/nfa/Atom.h"
#include "moonshine/lexer/dfa/DFA.h"
#include "moonshine/lexer/TokenType.h"

#include <memory>
#include <set>
#include <unordered_set>
#include <map>
#include <utility>
#include <string>

namespace moonshine { namespace nfa {

typedef std::vector<State>::size_type size_t;

class NFA
{
public:
    NFA();
    explicit NFA(const Atom& atom);
    explicit NFA(const char& character);
    static NFA str(const char* string);

    NFA kleene();
    NFA optional();

    dfa::DFA powerset();

    template<typename Iter>
    std::set<size_t> epsilonClosure(Iter begin, Iter end);
    template<typename Iter>
    std::set<size_t> move(Iter begin, Iter end, const char& symbol);
    bool isFinal(const size_t& index) const;
    NFA& token(const TokenType& token);

    void graphviz() const;

    NFA operator&(const NFA& rhs) const;
    NFA operator|(const NFA& rhs) const;
private:
    std::vector<State> states_;
    size_t start_;
    std::set<size_t> final_;
    std::multimap<size_t, std::pair<Atom, size_t>> transitions_;
    std::unordered_set<char> alphabet_;
};

}}