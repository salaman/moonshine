#include "Node.h"

#include <vector>
#include <stdexcept>
#include <iostream>

namespace moonshine { namespace ast {

void Node::makeSiblings(std::unique_ptr<Node> y)
{
    // find the rightmode node in this list
    Node* xsibs = this;

    while (xsibs->rightSib_ != nullptr) {
        xsibs = xsibs->rightSib_.get();
    }

    // join the lists
    Node* ysibs = y.get();

    // transfer ownership from the previous parent_ to this node
    // (for memory management, we acquire pointer ownership)
    xsibs->rightSib_ = std::move(y);

    // set pointers for the new siblings
    ysibs->leftmostSib_ = xsibs->leftmostSib_;
    ysibs->parent_ = xsibs->parent_;

    // update all new child's parents and leftmostSibs
    while (ysibs->rightSib_ != nullptr) {
        ysibs = ysibs->rightSib_.get();
        ysibs->leftmostSib_ = xsibs->leftmostSib_;
        ysibs->parent_ = xsibs->parent_;
    }
}

void Node::adoptChildren(std::unique_ptr<Node> y)
{
    if (leftmostChild_ != nullptr) {
        leftmostChild_->makeSiblings(std::move(y));
    } else {
        Node* ysibs = y.get();

        // transfer ownership of children from old parent_, after a gruesome custody battle
        // (for memory management, we acquire pointer ownership)
        leftmostChild_ = std::move(y);

        while (ysibs != nullptr) {
            ysibs->parent_ = this;
            ysibs = ysibs->rightSib_.get();
        }
    }
}

std::unique_ptr<Node> Node::makeNode(const std::string& name, Token* op)
{
    Node* node = nullptr;

    if (name == "addOp") {
        node = new AddOp(op);
    } else if (name == "relOp") {
        node = new RelOp(op);
    } else if (name == "num") {
        node = new Num(op);
    } else if (name == "id") {
        node = new Id(op);
    } else if (name == "multOp") {
        node = new MultOp(op);
    } else if (name == "ifStat") {
        node = new IfStat();
    } else if (name == "assignStat") {
        node = new AssignStat();
    } else if (name == "not") {
        node = new Not();
    } else if (name == "getStat") {
        node = new GetStat();
    } else if (name == "putStat") {
        node = new PutStat();
    } else if (name == "forStat") {
        node = new ForStat();
    } else if (name == "returnStat") {
        node = new ReturnStat();
    } else if (name == "sign") {
        node = new Sign(op);
    } else if (name == "statBlock") {
        node = new StatBlock();
    } else {
        throw std::runtime_error(std::string("Unexpected token encountered while creating AST node: ") + TokenName[op->type]);
    }

    //switch (op->type) {
    //    case TokenType::T_PLUS:
    //    case TokenType::T_MINUS:
    //    case TokenType::T_OR:
    //        node = new AddOp(op);
    //        break;
    //    case TokenType::T_MUL:
    //    case TokenType::T_DIV:
    //    case TokenType::T_AND:
    //        node = new MultOp(op);
    //        break;
    //    case TokenType::T_IS_EQUAL:
    //    case TokenType::T_IS_NOT_EQUAL:
    //    case TokenType::T_IS_SMALLER:
    //    case TokenType::T_IS_SMALLER_OR_EQUAL:
    //    case TokenType::T_IS_GREATER:
    //    case TokenType::T_IS_GREATER_OR_EQUAL:
    //        node = new RelOp(op);
    //        break;
    //    case TokenType::T_IDENTIFIER:
    //        node = new Id(op);
    //        break;
    //    case TokenType::T_INTEGER_LITERAL:
    //    case TokenType::T_FLOAT_LITERAL:
    //        node = new Num(op);
    //        break;
    //    case TokenType::T_IF:
    //        node = new IfStat(op);
    //        break;
    //    case TokenType::T_INT:
    //    case TokenType::T_FLOAT:
    //        node = new IfStat(op);
    //        break;
    //    default:
    //        throw std::runtime_error(std::string("Unexpected token encountered while creating AST node: ") + TokenName[op->type]);
    //}

    node->leftmostSib_ = node;

    return std::unique_ptr<Node>{node};
}

template<typename... Args>
void Node::makeFamily(std::unique_ptr<Node> op, Args... args)
{
    std::vector<std::unique_ptr<Node>> kids = {args...};

    for (auto& kid : kids) {
        op->adoptChildren(std::move(kid));
    }
}

Node* Node::child() const
{
    return leftmostChild_.get();
}

Node* Node::next() const
{
    return rightSib_.get();
}

void Node::print() const
{
    std::cout << name();

    auto ptr = child();

    if (ptr != nullptr) {
        std::cout << '{';

        while (ptr != nullptr) {
            ptr->print();
            ptr = ptr->next();
            if (ptr != nullptr) {
                std::cout << ' ';
            }
        }

        std::cout << '}';
    }
}

void Leaf::print() const
{
    Node::print();

    if (token_ != nullptr) {
        std::cout << '(' << token_->value << ')';
    }
}

}}