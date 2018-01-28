#include "Lexer.h"

#include "moonshine/lexer/nfa/NFA.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <stdexcept>

namespace moonshine {

Lexer::Lexer()
{
    using namespace nfa;

    // build an nfa to accept language
    NFA idToken = NFA(Atom::letter()) & NFA(Atom::alphanum()).kleene();

    NFA integerToken = (NFA(Atom::nonzero()) & NFA(Atom::digit()).kleene())
                       | NFA('0');

    NFA fractionAtom = (NFA('.') & NFA(Atom::digit()).kleene() & NFA(Atom::nonzero()))
                       | NFA::str(".0");

    NFA floatToken = integerToken & fractionAtom;

    NFA equalsToken = NFA::str("==");
    NFA andToken = NFA::str("and");
    NFA notToken = NFA::str("not");
    NFA semicolonToken = NFA(';');

    NFA nfa = andToken.attachToken(TokenType::T_AND)
              | notToken.attachToken(TokenType::T_NOT)
              | idToken.attachToken(TokenType::T_IDENTIFIER)
              | integerToken.attachToken(TokenType::T_INTEGER_LITERAL)
              | floatToken.attachToken(TokenType::T_FLOAT_LITERAL)
              | equalsToken.attachToken(TokenType::T_EQUAL)
              | semicolonToken.attachToken(TokenType::T_SEMICOLON);

    // convert nfa to dfa
    dfa::DFA dfa = nfa.powerset();

    // simulate various test strings
    //std::vector<std::string> tests = {
    //    "",
    //    "abc",
    //    "ab_c",
    //    "0abc",
    //    "0",
    //    "1",
    //    "123",
    //    "123.",
    //    "123.0",
    //    "=",
    //    "==",
    //    "a",
    //    "an",
    //    "and",
    //    "not",
    //    ";",
    //};
    //
    //for (const auto& s : tests) {
    //    dfa::DFASimulator simulator(dfa);
    //
    //    std::for_each(s.cbegin(), s.cend(), [&simulator](const char& c) {
    //        simulator.move(c);
    //    });
    //
    //    std::cout << s << ": " << (simulator.accepted() ? simulator.token() : "n/a") << std::endl;
    //}

    dfa_ = std::make_unique<dfa::DFASimulator>(dfa);
}

void Lexer::startLexing(std::istream* stream)
{
    stream_ = stream;
    readNextChar();
}

Token* Lexer::getNextToken()
{
    TokenType lastTokenType = TokenType::T_NONE;
    TokenType lastRecoverableTokenType = TokenType::T_NONE;
    std::string value;
    std::string::size_type lastRecoverableIndex = 0;

    dfa_->reset();

    while (true) {
        dfa_->move(buf_);

        if (dfa_->halted()) {
            break;
        } else {
            value.push_back(buf_);

            if (dfa_->accepted()) {
                lastTokenType = lastRecoverableTokenType = dfa_->token();
                lastRecoverableIndex = value.size();
            } else {
                lastTokenType = TokenType::T_NONE;
            }

            //lastTokenType = dfa_->accepted() ? dfa_->token() : T_NONE;
        }

        //if (stream_->eof()) {
        //    break;
        //}

        readNextChar();

        if (stream_->fail()) {
            throw std::runtime_error("Unexpected error in input stream");
        }
    }

    Token* token = nullptr;

    if (lastTokenType != TokenType::T_NONE) {
        token = new Token(lastTokenType, value, 0);
    } else if (lastRecoverableTokenType != TokenType::T_NONE) {
        token = new Token(lastRecoverableTokenType, value.substr(0, lastRecoverableIndex), 0);
        //buffer_ += value.substr(lastRecoverableIndex);
        for (auto i = value.size(); i > lastRecoverableIndex; --i, stream_->unget());
        readNextChar();
    } else if (buf_ != '\0') {
        // TODO - error
        token = new Token(TokenType::T_NONE, value + buf_, 0);
        readNextChar();
    }

    return token;
}

void Lexer::readNextChar()
{
    if (!buffer_.empty()) {
        buf_ = buffer_.front();
        buffer_.erase(0);
    } else if (!(*stream_ >> std::ws).eof()) {
        stream_->get(buf_);
    } else {
        buf_ = '\0';
    }
}

}