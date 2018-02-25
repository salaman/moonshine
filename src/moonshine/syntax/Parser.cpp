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

    //std::cout << std::setw(9) << std::left << "Input"
    //          << "Production" << std::endl;

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

                //std::cout << std::setw(9) << std::left << TokenName[a->type]
                //          << std::setw(17) << grammar_.tokenName(x) << " -> ";
                //for (const auto& i : p.rhs) std::cout << grammar_.tokenName(i) << " ";
                //if (p.rhs.empty()) std::cout << "ε";
                //std::cout << std::endl;

                //printProduction(x, p);

                stack_.pop_back();
                inverseRHSMultiplePush(p.rhs);
            } else {
                // skipErrors() ; error = true
                error = true;
                throw std::runtime_error("err 2");
            }
            std::cout << std::endl;
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
    std::ostringstream oss;

    for (const auto& t : parsedTokens_) {
        oss << TokenName[t->type] << ' ';
    }

    std::for_each(stack_.rbegin(), --stack_.rend(), [this, &oss](const GrammarToken& t) {
        oss << grammar_.tokenName(t) << ' ';
    });

    std::cout << std::setw(80) << std::left << oss.str();
}

void Parser::printSentencialForm(const GrammarToken& token, const Production& production)
{
    printSentencialForm();

    std::cout << grammar_.tokenName(token) << " -> ";

    for (const auto& i : production.rhs) std::cout << grammar_.tokenName(i) << " ";
    if (production.rhs.empty()) std::cout << "ε";
}

}}
