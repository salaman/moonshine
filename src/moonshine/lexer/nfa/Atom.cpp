#include "Atom.h"

namespace moonshine { namespace nfa {

Atom::Atom()
    : Atom("")
{
}

Atom::Atom(const std::string& label)
    : characters_(), label_(label)
{
}

Atom Atom::epsilon()
{
    static Atom A("ɛ");
    return A;
}

Atom Atom::letter()
{
    Atom temp("letter");

    for (char c = 'a'; c <= 'z'; ++c) {
        temp.characters_.insert(c);
    }

    for (char c = 'A'; c <= 'Z'; ++c) {
        temp.characters_.insert(c);
    }

    return temp;
}

Atom Atom::digit()
{
    Atom temp("digit");

    for (char c = '0'; c <= '9'; ++c) {
        temp.characters_.insert(c);
    }

    temp.label_ = "l";

    return temp;
}

Atom Atom::nonzero()
{
    Atom temp("nonzero");

    for (char c = '1'; c <= '9'; ++c) {
        temp.characters_.insert(c);
    }

    return temp;
}

Atom Atom::alphanum()
{
    static Atom a = Atom::letter() + Atom::digit() + Atom::ch('_');
    a.label_ = "alphanum";
    return a;
}

Atom Atom::ws()
{
    static Atom a = Atom::str(" \n\r\t\b\v\f");
    a.label_ = "white";
    return a;
}

Atom Atom::ch(const char& character)
{
    Atom temp({character});

    temp.characters_.insert(character);

    return temp;
}

Atom Atom::str(const char* string)
{
    Atom temp(string);

    for (int i = 0; string[i] != '\0'; ++i) {
        temp += Atom::ch(string[i]);
    }

    return temp;
}

bool Atom::matches() const
{
    return characters_.empty();
}

bool Atom::matches(char character) const
{
    return characters_.find(character) != characters_.end();
}

const std::set<char>& Atom::characters() const
{
    return characters_;
}

Atom Atom::operator+(const Atom& rhs)
{
    Atom temp(*this);

    temp += rhs;

    return temp;
}

Atom& Atom::operator+=(const Atom& rhs)
{
    characters_.insert(rhs.characters_.cbegin(), rhs.characters_.cend());

    return *this;
}

const std::string& Atom::label() const
{
    return label_;
}

}}