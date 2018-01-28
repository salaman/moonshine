#pragma once

#include "moonshine/lexer/Token.h"
#include "moonshine/lexer/TokenType.h"
#include "moonshine/lexer/ParseError.h"
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
    const std::vector<ParseError>& getErrors() const;
private:
    char character_;
    std::unique_ptr<dfa::DFASimulator> dfa_;
    std::istream* stream_;
    std::string buffer_;
    unsigned long position_;
    std::vector<ParseError> errors_;

    bool readNextChar();
    bool eof();
    bool isWhitespace(const char& c);
};

}