#pragma once

#include "moonshine/lexer/Lexer.h"
#include "moonshine/lexer/Token.h"
#include "moonshine/syntax/Grammar.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/syntax/ParseError.h"

#include <vector>
#include <memory>
#include <ostream>

namespace moonshine { namespace syntax {

class Parser
{
public:
    explicit Parser(const Grammar& grammar);

    std::unique_ptr<ast::Node> parse(Lexer* lex, std::ostream* output);
    const std::vector<ParseError>& getErrors() const;
    void setAnsi(const bool& ansi);
private:
    const Grammar grammar_;
    std::vector<GrammarToken> stack_;
    std::vector<std::unique_ptr<ast::Node>> semanticStack_;
    std::vector<std::shared_ptr<Token>> parsedTokens_;
    std::vector<ParseError> errors_;
    bool ansi_ = true;

    void inverseRHSMultiplePush(const std::vector<GrammarToken>& tokens);
    void skipErrors(Lexer* lex, std::shared_ptr<Token>& a, const bool& isPopError);

    void printSentencialForm(std::ostream* output);
    void printSentencialForm(std::ostream* output, const GrammarToken& token, const Production& production);
    void printSemanticStack(std::ostream* output);
};

}}