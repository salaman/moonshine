#include "Atom.h"

namespace moonshine { namespace nfa {

Atom::Atom()
    : characters_()
{
}

Atom Atom::epsilon()
{
    return Atom();
}

Atom Atom::letter()
{
    // TODO: return cached copies

    Atom temp;

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
    Atom temp;

    for (char c = '0'; c <= '9'; ++c) {
        temp.characters_.insert(c);
    }

    return temp;
}

Atom Atom::nonzero()
{
    Atom temp;

    for (char c = '1'; c <= '9'; ++c) {
        temp.characters_.insert(c);
    }

    return temp;
}

Atom Atom::alphanum()
{
    return Atom::letter() + Atom::digit() + Atom::ch('_');
}

Atom Atom::ch(const char& character)
{
    Atom temp;

    temp.characters_.insert(character);

    return temp;
}

Atom Atom::str(const char* string)
{
    Atom temp;

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

}}