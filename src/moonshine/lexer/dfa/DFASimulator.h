#pragma once

#include "moonshine/lexer/dfa/DFA.h"

namespace dfa {

class DFASimulator
{
public:
    explicit DFASimulator(const DFA& dfa);
    void move(const char& character);
    bool accepted() const;
private:
    const DFA dfa_;
    size_t currentState_;
    bool halted_;
};

}