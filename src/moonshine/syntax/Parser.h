#pragma once

#include "moonshine/lexer/Lexer.h"
#include "moonshine/lexer/Token.h"
#include "moonshine/syntax/Grammar.h"
#include "moonshine/syntax/Node.h"

#include <vector>
#include <memory>

namespace moonshine { namespace syntax {

class Parser
{
public:
    explicit Parser(const Grammar& grammar);

    bool parse(Lexer* lex);
    void inverseRHSMultiplePush(const std::vector<GrammarToken>& tokens);
private:
    const Grammar grammar_;
    std::vector<GrammarToken> stack_;
    std::vector<std::unique_ptr<ast::Node>> semanticStack_;
    std::vector<Token*> parsedTokens_;

    void printSentencialForm();
    void printSentencialForm(const GrammarToken& token, const Production& production);
    void printSemanticStack();
};

}}