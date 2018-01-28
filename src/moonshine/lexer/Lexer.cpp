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

    dfa_->reset();

    //readNextChar();
    shouldReadChar_ = false;
    finished_ = false;
    hitEOF_ = false;
    //test_ = static_cast<unsigned int>(-1);
    position_ = 0u;
    line_ = 0u;
    //buf_ = ' ';
}

Token* Lexer::getNextToken()
{
    if (stream_->bad()) {
        return nullptr;
    }

    TokenType lastTokenType = TokenType::T_NONE;
    std::string value;
    std::string::size_type tokenLength = 0u;

    unsigned long tokenStartPos = 0u;
    unsigned long tokenStartLine = 0u;
    unsigned long tokenStartColumn = 0u;

    Token* token = nullptr;

    while (!eof() && token == nullptr) {

        bool newToken = true;

        while (!dfa_->halted()) {

            if (!readNextChar()) {
                // EOF
                break;
            }

            value.push_back(buf_);
            dfa_->move(buf_);

            if (dfa_->accepted()) {
                if (newToken) {
                    tokenStartPos = position_ - buffer_.size();
                    tokenStartLine = line_;
                    tokenStartColumn = column_;
                    newToken = false;
                }

                lastTokenType = dfa_->token();
                tokenLength = value.size();
            }

        }

        // return last token handled & reset dfa for next token

        dfa_->reset();

        if (tokenLength != 0) {
            token = new Token(lastTokenType, value.substr(0, tokenLength), tokenStartPos, tokenStartLine, tokenStartColumn);
            buffer_ += value.substr(tokenLength);
        } else {
            // TODO ERROR
            std::cout << "ERROR (" << value << ')' << std::endl;
        }

        value.clear();
    }

    return token;
}

bool Lexer::readNextChar()
{
    // try reading from the character buffer first
    if (!buffer_.empty()) {
        buf_ = buffer_.front();
        buffer_.erase(buffer_.begin());
        return true;
    }

    bool carriageReturn = false;

    // fetch a character from the input stream
    while (!stream_->eof()) {
        stream_->get(buf_);
        ++position_;

        // handle line count
        if (buf_ == '\r') {
            // if we get an \r, don't immediately increment the line count since a \n could follow
            carriageReturn = true;
            continue; // optim
        } else if (buf_ == '\n') {
            // when we get an \n, always increment the line count
            ++line_;
            column_ = 0u;
            continue; // optim
        } else if (carriageReturn) {
            // we only got an \r (Windows), increment line count and stop processing the newline
            ++line_;
            column_ = 0u;
            carriageReturn = false;
        }

        // skip whitespace
        if (!isWhitespace(buf_)) {
            break;
        }
    }

    // don't count eof as a position
    if (stream_->eof()) {
        --position_;
        return false;
    }

    return true;
}

//bool Lexer::readNextChar()
//{
//
//    if (!buffer_.empty()) {
//        buf_ = buffer_.front();
//        buffer_.erase(buffer_.begin());
//        //--test_;
//        return true;
//    } else if (!(*stream_ >> std::ws).eof()) {
//        pos_ = stream_->tellg();
//        stream_->get(buf_);
//        //++test_;
//        return true;
//    }
//
//    return false;
//}

bool Lexer::eof()
{
    return buffer_.empty() && stream_->eof();
}

bool Lexer::isWhitespace(const char& c)
{
    return (' ' == c) || ('\n' == c) ||
           ('\r' == c) || ('\t' == c) ||
           ('\b' == c) || ('\v' == c) ||
           ('\f' == c);
}

bool Lexer::isNewline(const char& c)
{
    return ('\r' == c) || ('\n' == c);
}

}