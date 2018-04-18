#include "Parser.h"

#include <algorithm>
#include <iostream>

namespace moonshine { namespace syntax {

Parser::Parser(const Grammar& grammar)
    : grammar_(grammar), stack_(), parsedTokens_()
{
}

std::unique_ptr<ast::Node> Parser::parse(Lexer* lex, std::ostream* output)
{
    bool error = false;

    stack_.emplace_back(GrammarTokenType::END, 0);
    stack_.push_back(grammar_.startToken());

    std::shared_ptr<Token> a(lex->getNextToken());
    Production p;

    while (stack_.back().type != GrammarTokenType::END) {
        const GrammarToken x = stack_.back();

        if (x.type == GrammarTokenType::TERMINAL) {

            /*
             * Terminal symbol
             */

            if (a == nullptr) {
                skipErrors(lex, a, true);
                error = true;
            } else if (x.value == static_cast<const int>(a->type)) {
                stack_.pop_back();
                parsedTokens_.emplace_back(a);
                a.reset(lex->getNextToken());
            } else {
                skipErrors(lex, a, p.isPopError);
                error = true;
            }

        } else if (x.type == GrammarTokenType::NON_TERMINAL) {

            /*
             * Non-terminal symbol
             */

            if (a == nullptr) {
                skipErrors(lex, a, true);
                error = true;
            } else if (!(p = grammar_(x, a->type)).isError()) {
                if (output) {
                    printSentencialForm(output, x, p);
                    printSemanticStack(output);
                }

                stack_.pop_back();
                inverseRHSMultiplePush(p.rhs);
            } else {
                skipErrors(lex, a, p.isPopError);
                error = true;
            }

        } else if (x.type == GrammarTokenType::SEMANTIC) {

            /*
             * Semantic symbol
             */

            stack_.pop_back();

            if (error) {
                // stop processing semantic stack on error
                //continue;
            }

            if (x.value == -1) {
                // pop
                semanticStack_.pop_back();
            } else if (x.value == 0) {
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
            } else if (x.parent < 0) {
                std::vector<std::unique_ptr<ast::Node>> children;

                auto it = semanticStack_.end();
                for (int i = 0; i < x.value; ++i, --it);

                semanticStack_.erase(it);
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

        }

    }

    if (output) {
        printSentencialForm(output);
        *output << std::endl;
        printSemanticStack(output);
    }

    if (/*error ||*/ stack_.size() != 1 || stack_.back().type != GrammarTokenType::END) {
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

void Parser::skipErrors(Lexer* lex, std::shared_ptr<Token>& a, const bool& isPopError)
{
    if (a) {
        errors_.emplace_back(ParseErrorType::E_UNEXPECTED_TOKEN, a);
    } else {
        errors_.emplace_back(ParseErrorType::E_UNEXPECTED_EOF, a);
    }

    if (isPopError) {
        stack_.pop_back();

        while (stack_.back().type == GrammarTokenType::SEMANTIC) {
            const GrammarToken x = stack_.back();
            stack_.pop_back();

            if (x.value == -1) {
                // pop
                semanticStack_.pop_back();
            } else if (x.value == 0) {
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
            } else if (x.parent < 0) {
                std::vector<std::unique_ptr<ast::Node>> children;

                auto it = semanticStack_.end();
                for (int i = 0; i < x.value; ++i, --it);

                semanticStack_.erase(it);
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
        }

    } else {
        a.reset(lex->getNextToken());

        while (a && grammar_(stack_.back(), a->type).isError()) {
            a.reset(lex->getNextToken());
        }
    }
}

void Parser::printSentencialForm(std::ostream* output)
{
    if (!output) {
        return;
    }

    for (const auto& t : parsedTokens_) {
        *output << TokenName[t->type] << ' ';
    }

    bool first = true;

    std::for_each(stack_.rbegin(), --stack_.rend(), [this, &first, &output](const GrammarToken& t) {
        if (first) {
            if (ansi_) *output << "\033[4m";
            first = false;
        }

        *output << grammar_.tokenName(t, ansi_);
        *output << (ansi_ ? "\033[0m" : "") << ' ';
    });
}

void Parser::printSentencialForm(std::ostream* output, const GrammarToken& token, const Production& production)
{
    if (!output) {
        return;
    }

    printSentencialForm(output);

    *output << std::endl << "↳ new production: " << grammar_.tokenName(token, ansi_) << " -> ";

    for (const auto& i : production.rhs) {
        *output << grammar_.tokenName(i, ansi_) << " ";
    }

    if (production.rhs.empty()) {
        *output << (ansi_ ? "\033[34m" : "") << "ε" << (ansi_ ? "\033[0m" : "");
    }

    *output << std::endl;
}

void Parser::printSemanticStack(std::ostream* output)
{
    if (!output) {
        return;
    }

    *output << "↳ semantic stack: ";

    for (const auto& i : semanticStack_) {
        i->print(output);
        *output << ' ';
    }

    *output << std::endl << std::endl;
}

void Parser::setAnsi(const bool& ansi)
{
    ansi_ = ansi;
}

const std::vector<ParseError>& Parser::getErrors() const
{
    return errors_;
}

}}
