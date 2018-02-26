#include "Parser.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace moonshine { namespace syntax {

Parser::Parser(const Grammar& grammar)
    : grammar_(grammar), stack_(), parsedTokens_()
{
}

std::unique_ptr<ast::Node> Parser::parse(Lexer* lex)
{
    bool error = false;

    stack_.emplace_back(GrammarTokenType::END, 0);
    stack_.push_back(grammar_.startToken());

    std::shared_ptr<Token> a(lex->getNextToken());

    while (stack_.back().type != GrammarTokenType::END) {
        const GrammarToken x = stack_.back();

        if (a != nullptr && x.type == GrammarTokenType::TERMINAL) {

            /*
             * Terminal symbol
             */

            if (x.value == static_cast<const int>(a->type)) {
                stack_.pop_back();
                //delete a;
                parsedTokens_.emplace_back(a);
                a.reset(lex->getNextToken());
            } else {
                // skipErrors()
            }

        } if (a != nullptr && x.type == GrammarTokenType::NON_TERMINAL) {

            /*
             * Non-terminal symbol
             */

            const Production p = grammar_(x, a->type);

            printSentencialForm(x, p);
            printSemanticStack();

            if (!p.isError()) {
                stack_.pop_back();
                inverseRHSMultiplePush(p.rhs);
            } else {
                error = true;
                throw std::runtime_error("err 1");
            }

        } else if (x.type == GrammarTokenType::SEMANTIC) {

            /*
             * Semantic symbol
             */

            stack_.pop_back();

            if (x.value == 0) {
                // makeNode() - semantic action to create a leaf node
                semanticStack_.emplace_back(ast::Node::makeNode(x.name, a));
            } else if (x.parent == 0) {
                // makeSiblings() - semantic action to join several AST nodes
                std::vector<std::unique_ptr<ast::Node>> children;

                for (int i = x.value; i > 1; --i) {
                    if (semanticStack_.empty()) {
                        throw std::runtime_error("Tried popping from empty semantic stack in makeSiblings");
                    } else {
                        children.push_back(std::move(semanticStack_.back()));
                    }

                    semanticStack_.pop_back();
                }

                for (auto i = children.rbegin(); i != children.rend(); ++i) {
                    semanticStack_.back()->adoptChildren(std::move(*i));
                }
            } else {
                // makeFamily() - semantic action to create a new AST hierarchy
                std::vector<std::unique_ptr<ast::Node>> children;
                std::unique_ptr<ast::Node> parent;

                for (int i = x.value; i > 0; --i) {
                    if (semanticStack_.empty()) {
                        throw std::runtime_error("Tried popping from empty semantic stack in makeFamily");
                    } else if (i == x.parent) {
                        parent = std::move(semanticStack_.back());
                    } else {
                        children.push_back(std::move(semanticStack_.back()));
                    }

                    semanticStack_.pop_back();
                }

                for (auto i = children.rbegin(); i != children.rend(); ++i) {
                    parent->adoptChildren(std::move(*i));
                }

                semanticStack_.push_back(std::move(parent));
            }

                // skipErrors() ; error = true
                error = true;
                throw std::runtime_error("err 2");
        }

    }

    printSentencialForm();

    if (error || stack_.size() != 1 || stack_.back().type != GrammarTokenType::END) {
        return nullptr;
    }

    std::unique_ptr<ast::Node> node = std::move(semanticStack_.back());
    semanticStack_.pop_back();

    return node;
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

void Parser::printSemanticStack()
{
    std::cout << "↳ semantic stack: ";

    for (const auto& i : semanticStack_) {
        i->print();
        std::cout << ' ';
    }

    std::cout << std::endl;
}

}}
