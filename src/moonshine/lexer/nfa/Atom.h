#pragma once

#include <set>
#include <string>

namespace moonshine { namespace nfa {

class Atom
{
public:
    static Atom epsilon();
    static Atom letter();
    static Atom digit();
    static Atom nonzero();
    static Atom alphanum();
    static Atom ws();
    static Atom ch(const char& character);
    static Atom str(const char* character);

    Atom();
    explicit Atom(const std::string& label);

    bool matches() const;
    bool matches(char character) const;
    const std::set<char>& characters() const;
    const std::string& label() const;

    Atom operator+(const Atom& rhs);
    Atom& operator+=(const Atom& rhs);
private:
    std::set<char> characters_;
    std::string label_;
};

}}