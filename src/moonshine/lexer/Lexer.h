#pragma once

#include "moonshine/lexer/Token.h"
#include "moonshine/lexer/TokenType.h"
#include "moonshine/lexer/dfa/DFASimulator.h"

#include <istream>
#include <memory>
#include <queue>

namespace moonshine {

class Lexer
{
public:
    Lexer();

    void startLexing(std::istream* stream);
    Token* getNextToken();
    //std::unique_ptr<Token> getNextToken();
private:
    char buf_;
    std::unique_ptr<dfa::DFASimulator> dfa_;
    std::istream* stream_;
    std::string buffer_;
    bool shouldReadChar_;
    bool finished_;
    bool hitEOF_;
    std::streampos pos_;

    unsigned long position_;
    unsigned long line_;
    unsigned long column_;

    bool readNextChar();
    bool eof();
    bool isWhitespace(const char& c);
    bool isNewline(const char& c);
};

}