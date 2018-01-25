#include <moonshine/lexer/nfa/NFA.h>
#include <moonshine/lexer/dfa/DFA.h>
#include <moonshine/lexer/dfa/DFASimulator.h>

#include <iostream>
#include <algorithm>
#include <string>

using namespace nfa;

int main()
{
    // build an nfa to accept language
    NFA idToken = NFA(Atom::letter()) & NFA(Atom::alphanum()).kleene();

    NFA integerToken = (NFA(Atom::nonzero()) & NFA(Atom::digit()).kleene())
                       | NFA('0');

    NFA fractionAtom = (NFA('.') & NFA(Atom::digit()).kleene() & NFA(Atom::nonzero()))
                       | (NFA('.') & NFA('0'));

    NFA floatToken = integerToken & fractionAtom;

    NFA equalsToken = NFA('=') & NFA('=');
    NFA andToken = NFA::str("and");
    NFA notToken = NFA::str("not");

    NFA nfa = idToken
              | integerToken
              | floatToken
              | equalsToken
              | notToken;

    // convert nfa to dfa
    dfa::DFA dfa = nfa.powerset();

    // simulate various test strings
    std::vector<std::string> tests = {
        "",
        "abc",
        "ab_c",
        "0abc",
        "0",
        "1",
        "123",
        "123.",
        "123.0",
        "=",
        "==",
        "a",
        "an",
        "and",
        "not",
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