#include "Parser.h"

#include <algorithm>

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

        if (/*a != nullptr &&*/ x.type == GrammarTokenType::TERMINAL) {

            /*
             * Terminal symbol
             */

            if (a == nullptr) {
                skipErrors(lex, a, true);
                error = true;
                continue;
            }

            if (x.value == static_cast<const int>(a->type)) {
                stack_.pop_back();
                //delete a;
                parsedTokens_.emplace_back(a);
                a.reset(lex->getNextToken());
            } else {
                //throw std::runtime_error("error");
                skipErrors(lex, a, p.isPopError);
                error = true;
            }

        } else if (/*a != nullptr &&*/ x.type == GrammarTokenType::NON_TERMINAL) {

            /*
             * Non-terminal symbol
             */

            if (a == nullptr) {
                skipErrors(lex, a, true);
                error = true;
                continue;
            }

            p = grammar_(x, a->type);

            if (output) {
                printSentencialForm(output, x, p);
                printSemanticStack(output);
            }

            if (!p.isError()) {
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

    if (error || stack_.size() != 1 || stack_.back().type != GrammarTokenType::END) {
        return std::move(semanticStack_.back());
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
    //std::cout << "*** syntax error at " << a->value << std::endl;

    if (isPopError) {
        stack_.pop_back();
    } else {
        a.reset(lex->getNextToken());
        //throw std::runtime_error("err 2");
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
            *output << "\033[4m";
            first = false;
        }

        *output << grammar_.tokenName(t) << "\033[0m ";
    });
}

void Parser::printSentencialForm(std::ostream* output, const GrammarToken& token, const Production& production)
{
    if (!output) {
        return;
    }

    printSentencialForm(output);

    *output << std::endl << "↳ new production: " << grammar_.tokenName(token) << " -> ";

    for (const auto& i : production.rhs) {
        *output << grammar_.tokenName(i) << " ";
    }

    if (production.rhs.empty()) {
        *output << "\033[34mε\033[0m";
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

}}
