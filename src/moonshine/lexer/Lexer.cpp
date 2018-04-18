#include "Lexer.h"

#include "moonshine/lexer/nfa/NFA.h"
#include "moonshine/lexer/nfa/Atom.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <stdexcept>

namespace moonshine {

Lexer::Lexer()
{
    using namespace nfa;

    /*
     * NFA language definition
     */

    NFA ws = NFA(Atom::ws()).optional();

    NFA idToken = NFA(Atom::letter()) & NFA(Atom::alphanum()).kleene();

    NFA integerToken = (NFA(Atom::nonzero()) & NFA(Atom::digit()).kleene())
                       | NFA('0');

    NFA fractionAtom = (NFA('.') & NFA(Atom::digit()).kleene() & NFA(Atom::nonzero()))
                       | NFA::str(".0");

    NFA exponentAtom = NFA('e') & (NFA('+') | NFA('-')).optional() & integerToken;

    NFA floatToken = integerToken & fractionAtom & exponentAtom.optional();

    NFA nfa = (
        // comparison operators
        NFA::str("==").token(TokenType::T_IS_EQUAL)
        | NFA::str("<>").token(TokenType::T_IS_NOT_EQUAL)
        | NFA::str("<=").token(TokenType::T_IS_SMALLER_OR_EQUAL)
        | (NFA('<') & ws).token(TokenType::T_IS_SMALLER)
        | NFA::str(">=").token(TokenType::T_IS_GREATER_OR_EQUAL)
        | (NFA('>') & ws).token(TokenType::T_IS_GREATER)

        // punctuation
        | NFA(';').token(TokenType::T_SEMICOLON)
        | NFA(',').token(TokenType::T_COMMA)
        | NFA('.').token(TokenType::T_PERIOD)
        | NFA::str("::").token(TokenType::T_DOUBLE_COLON)
        | (NFA(':') & ws).token(TokenType::T_COLON)
        | NFA('(').token(TokenType::T_OPEN_PARENTHESIS)
        | NFA(')').token(TokenType::T_CLOSE_PARENTHESIS)
        | NFA('{').token(TokenType::T_OPEN_BRACE)
        | NFA('}').token(TokenType::T_CLOSE_BRACE)
        | NFA('[').token(TokenType::T_OPEN_BRACKET)
        | NFA(']').token(TokenType::T_CLOSE_BRACKET)

        // arithmetic operators
        | NFA('+').token(TokenType::T_PLUS)
        | NFA('-').token(TokenType::T_MINUS)
        | NFA('*').token(TokenType::T_MUL)
        | NFA('/').token(TokenType::T_DIV)

        // assignment operators
        | (NFA('=') & ws).token(TokenType::T_EQUAL)

        // control flow keywords
        | (NFA::str("and") & ws).token(TokenType::T_AND)
        | (NFA::str("not") & ws).token(TokenType::T_NOT)
        | (NFA::str("or") & ws).token(TokenType::T_OR)

        // control flow keywords
        | (NFA::str("if") & ws).token(TokenType::T_IF)
        | (NFA::str("then") & ws).token(TokenType::T_THEN)
        | (NFA::str("else") & ws).token(TokenType::T_ELSE)
        | (NFA::str("for") & ws).token(TokenType::T_FOR)
        | (NFA::str("get") & ws).token(TokenType::T_GET)
        | (NFA::str("put") & ws).token(TokenType::T_PUT)
        | (NFA::str("return") & ws).token(TokenType::T_RETURN)
        | (NFA::str("program") & ws).token(TokenType::T_PROGRAM)

        // type declarations
        | (NFA::str("class") & ws).token(TokenType::T_CLASS)
        | (NFA::str("int") & ws).token(TokenType::T_INT)
        | (NFA::str("float") & ws).token(TokenType::T_FLOAT)

        // type atoms
        | (idToken & ws).token(TokenType::T_IDENTIFIER)
        | (integerToken & ws).token(TokenType::T_INTEGER_LITERAL)
        | (floatToken & ws).token(TokenType::T_FLOAT_LITERAL)
    );

    // convert nfa to dfa
    dfa::DFA dfa = nfa.powerset();

    //nfa.graphviz();

    dfa_ = std::unique_ptr<dfa::DFASimulator>(new dfa::DFASimulator(dfa));
}

void Lexer::startLexing(std::istream* stream, std::ostream* output)
{
    stream_ = stream;
    output_ = output;

    dfa_->reset();

    position_ = static_cast<unsigned long>(-1);
    errors_ = std::vector<ParseError>();
}

Token* Lexer::getNextToken()
{
    if (stream_->bad()) {
        return nullptr;
    }

    Token* token = nullptr;
    TokenType lastTokenType = TokenType::T_NONE;

    // current read buffer
    std::string value;
    // token's start index within value
    std::string::size_type tokenStartIndex = 0u;
    // token's end index within value
    std::string::size_type tokenEndIndex = 0u;
    // token's global position in stream
    unsigned long tokenPosition = 0u;
    unsigned long errorStart = 0u;

    auto reset = [&](bool& newToken) {
        value.pop_back();
        newToken = true;
        lastTokenType = TokenType::T_NONE;
        tokenStartIndex = 0u;
        tokenEndIndex = 0u;
        tokenPosition = 0u;
        dfa_->reset();
    };

    /*
     * token processing loop
     */

    while (!eof() && token == nullptr) {

        bool newToken = true;

        /*
         * DFA simulation loop
         */

        while (!dfa_->halted()) {

            // read the next input stream character
            if (!readNextChar()) {
                // reached EOF
                break;
            }

            // check for line and block comments
            if (value.back() == '/') {
                bool needsReset = character_ == '/' || character_ == '*';
                bool terminatedComment = true;

                if (character_ == '/') {
                    // consume all input until a newline
                    readUntil('\n');
                } else if (character_ == '*') {
                    // consume all input until a */
                    do {
                        readUntil('*');
                    } while ((terminatedComment = readNextChar()) && character_ != '/');
                }

                if (!terminatedComment) {
                    errors_.emplace_back(ParseErrorType::E_UNTERMINATED_COMMENT, "/*", tokenPosition);
                }

                // we've just processed a comment and should reset our state and find a new token
                if (needsReset) {
                    reset(newToken);
                    continue;
                }
            }

            // to enable matching of whitespace in the grammar (ie. for separating keywords and idents),
            // we'll selectively ignore whitespace only if there are no DFA transitions available for it
            if (nfa::Atom::ws().matches(character_)) {
                if (dfa_->hasMove(character_)) {
                    // this whitespace matters for rule selection, consume it
                    dfa_->move(character_);
                } else {
                    // ignore this whitespace & carry on
                    continue;
                }
            }

            // if we're reading a new (potential) token, set its start index within value to the current char
            if (newToken) {
                tokenPosition = position_ - buffer_.size();
                tokenStartIndex = value.size();
                newToken = false;
            }

            // simulate!
            dfa_->move(character_);
            value.push_back(character_);

            // if this is an accepting state, set the current token type that we're matching,
            // and update the end position within value to reflect the match
            if (dfa_->accepted()) {
                lastTokenType = dfa_->token();
                tokenEndIndex = value.size();
            }
        }

        // reset dfa for next token
        dfa_->reset();

        // return last token handled
        if (lastTokenType != TokenType::T_NONE) {
            token = new Token(lastTokenType, value.substr(tokenStartIndex, tokenEndIndex - tokenStartIndex), tokenPosition);

            // if this token isn't at the beginning of the value buffer, it means we have some error input
            if (tokenStartIndex != 0) {
                errors_.emplace_back(ParseErrorType::E_INVALID_CHARACTERS, value.substr(0, tokenStartIndex), errorStart);
                errorStart = 0u;
            }

            // store anything we've read past the end of the token into our input buffer for processing next call
            buffer_ += value.substr(tokenEndIndex);
            value.clear();
        } else if (value.size() == 1) {
            // store the start position of the error for future use since this is a new one
            errorStart = position_;
        }

    }

    // we broke out of the token parse loop without fully using up our value,
    // hence we reached eof without parsing a valid token
    if (!value.empty()) {
        errors_.emplace_back(ParseErrorType::E_INVALID_CHARACTERS, value, errorStart);
    }

    if (token && output_) {
        if (atocc) {
            *output_ << token->name() << ' ';
        } else {
            *output_ << token->name() << " \"" << token->value << "\" " << token->position << std::endl;
        }
    }

    return token;
}

bool Lexer::readNextChar()
{
    // try reading from the character buffer first
    if (!buffer_.empty()) {
        character_ = buffer_.front();
        buffer_.erase(buffer_.begin());
        return true;
    }

    // fetch a character from the input stream
    stream_->get(character_);
    ++position_;

    // don't count eof as a position
    if (stream_->eof()) {
        --position_;
        return false;
    }

    return true;
}

bool Lexer::eof()
{
    return buffer_.empty() && stream_->eof();
}

const std::vector<ParseError>& Lexer::getErrors() const
{
    return errors_;
}

bool Lexer::readUntil(const char& c)
{
    bool ret;

    do {
        ret = readNextChar();
    } while (ret && character_ != c);

    return ret;
}

}
