#pragma once

#include "moonshine/lexer/dfa/DFA.h"
#include "moonshine/lexer/TokenType.h"

#include <string>

namespace moonshine { namespace dfa {

class DFASimulator
{
public:
    explicit DFASimulator(const DFA& dfa);
    bool hasMove(const char& character);
    void move(const char& character);
    bool accepted() const;
    bool halted() const;
    void reset();
    TokenType token() const;
private:
    const DFA dfa_;
    size_t currentState_;
    bool halted_;
};

}}
