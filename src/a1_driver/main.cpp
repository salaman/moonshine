#include <moonshine/lexer/nfa/NFA.h>
#include <moonshine/lexer/dfa/DFA.h>
#include <moonshine/lexer/dfa/DFASimulator.h>

#include <iostream>
#include <algorithm>
#include <string>

using namespace nfa;

int main()
{
    std::cout << "Hello, World!" << std::endl;

    //TokenBuilder builder = TokenBuilder::define(1)
    //        .with([](int& test) {
    //            test = 10;
    //        });

    // build an nfa to accept abc
    //NFA nfa = (NFA(Atom::ch('a')) & NFA(Atom::ch('b')) & NFA(Atom::ch('c')))
    //          | (NFA(Atom::ch('c')) & NFA(Atom::ch('b')) & NFA(Atom::ch('a')))
    //          | (NFA(Atom::digit()) & NFA(Atom::ch('b')) & NFA(Atom::nonzero()));

    NFA nfa = NFA(Atom::ch('a')) & NFA(Atom::ch('b')) & NFA(Atom::ch('c'));

    // convert nfa to dfa
    dfa::DFA dfa = nfa.powerset();

    // simulate various test strings
    std::vector<std::string> tests = {
        //"a",
        //"ab",
        //"abc",
        //"abcd",
        "abcc",
        //"abd",
        //"",
        //"c",
        //"cb",
        //"cba",
        //"ba",
        //"ca",
        //"0",
        //"0b",
        //"1b",
        //"0b1",
        //"0b9",
        //"0b0",
        //"1b0",
        //"0c1",
        //"1b11",
        //"1b1b",
    };

    for (const auto& s : tests) {
        dfa::DFASimulator simulator(dfa);

        std::for_each(s.cbegin(), s.cend(), [&simulator](const char& c) {
            simulator.move(c);
        });

        std::cout << s << ": " << (simulator.accepted() ? "yes" : "no") << std::endl;
    }

    return 0;
}