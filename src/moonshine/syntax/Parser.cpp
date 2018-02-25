#include "Parser.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace moonshine { namespace syntax {

Parser::Parser(const Grammar& grammar)
    : grammar_(grammar), stack_(), parsedTokens_()
{
}

bool Parser::parse(Lexer* lex)
{
    bool error = false;

    stack_.emplace_back(GrammarTokenType::END, 0);
    stack_.push_back(grammar_.startToken());

    Token* a = lex->getNextToken();

    while (a != nullptr && stack_.back().type != GrammarTokenType::END) {
        const GrammarToken x = stack_.back();

        if (x.type == GrammarTokenType::TERMINAL) {
            if (x.value == static_cast<const int>(a->type)) {
                stack_.pop_back();
                //delete a;
                parsedTokens_.emplace_back(a);
                a = lex->getNextToken();
            } else {
                // skipErrors()
                error = true;
                throw std::runtime_error("err 1");
            }
        } else {
            const Production p = grammar_(x, a->type);

            printSentencialForm(x, p);

            if (!p.isError()) {
                stack_.pop_back();
                inverseRHSMultiplePush(p.rhs);
            } else {
                // skipErrors() ; error = true
                error = true;
                throw std::runtime_error("err 2");
            }
        }

    }

    printSentencialForm();

    if (a != nullptr) {
        delete a;
        a = nullptr;
    }

    if (error || stack_.size() != 1 || stack_.back().type != GrammarTokenType::END) {
        return false;
    }

    return true;
}

void Parser::inverseRHSMultiplePush(const std::vector<GrammarToken>& tokens)
{
    std::for_each(tokens.rbegin(), tokens.rend(), [this](const GrammarToken& t) {
        stack_.push_back(t);
    });
}

void Parser::printSentencialForm()
{
    for (const auto& t : parsedTokens_) {
        std::cout << TokenName[t->type] << ' ';
    }

    bool first = true;

    std::for_each(stack_.rbegin(), --stack_.rend(), [this, &first](const GrammarToken& t) {
        if (first) {
            std::cout << "\033[4m";
            first = false;
        }

        std::cout << grammar_.tokenName(t) << "\033[0m ";
    });
}

void Parser::printSentencialForm(const GrammarToken& token, const Production& production)
{
    printSentencialForm();

    std::cout << std::endl << "↳ " << grammar_.tokenName(token) << " → ";

    for (const auto& i : production.rhs) {
        std::cout << grammar_.tokenName(i) << " ";
    }

    if (production.rhs.empty()) {
        std::cout << "\033[34mε\033[0m";
    }

    std::cout << std::endl;
}

}}
