#pragma once

#include <set>

class Atom
{
public:
    static Atom epsilon();
    static Atom letter();
    static Atom digit();
    static Atom nonzero();
    static Atom alphanum();
    static Atom ch(char character);

    Atom();

    bool matches() const;
    bool matches(char character) const;
    const std::set<char>& characters() const;

    Atom operator+(const Atom& rhs);
private:
    std::set<char> characters_;
};
