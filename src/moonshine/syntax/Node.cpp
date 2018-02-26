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

std::unique_ptr<Node> Node::makeNode(const std::string& name, std::shared_ptr<Token>& op)
{
    Node* node = nullptr;

    // lazy man's factory construction feat. macro abuse
    #define AST(NAME) if (name == #NAME) { node = new NAME(); goto found; }
    #define AST_LEAF(NAME) if (name == #NAME) { node = new NAME(op); goto found; }

    AST(nul);
    AST_LEAF(relOp);
    AST_LEAF(addOp);
    AST_LEAF(multOp);
    AST_LEAF(num);
    AST_LEAF(id);
    AST_LEAF(type);
    AST_LEAF(sign);
    AST(getStat);
    AST(putStat);
    AST(returnStat);
    AST(forStat);
    AST(notFactor);
    AST(ifStat);
    AST(assignStat);
    AST(statBlock);
    AST(prog);
    AST(classList);
    AST(funcDefList);
    AST(classDecl);
    AST(funcDef);
    AST(inherList);
    AST(membList);
    AST(varDecl);
    AST(fparamList);
    AST(dimList);
    AST(fparam);
    AST(var);
    AST(dataMember);
    AST(fCall);
    AST(indexList);
    AST(aParams);

    #undef AST
    #undef AST_LEAF

    throw std::runtime_error(std::string("Unexpected token encountered while creating AST node: ") + name);

found:
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