#include "Node.h"

#include "moonshine/Visitor.h"

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
    #define AST(NAME) if (name == #NAME) { node = new ast::NAME(); goto found; }
    #define AST_LEAF(NAME) if (name == #NAME) { node = new ast::NAME(op); goto found; }

    #include "ast_nodes.h"

    #undef AST
    #undef AST_LEAF

    throw std::runtime_error(std::string("Unexpected token encountered while creating AST node: ") + name);

found:
    node->leftmostSib_ = node;

    return std::unique_ptr<Node>{node};
}

Node* Node::parent() const
{
    return parent_;
}

Node* Node::child() const
{
    return leftmostChild_.get();
}

Node* Node::child(const unsigned int& index) const
{
    Node* xsibs = child();

    for (unsigned int i = 0; i < index && xsibs != nullptr; ++i) {
        xsibs = xsibs->next();
    }

    return xsibs;
}

Node* Node::rightmostChild() const
{
    Node* xsibs = child();

    while (xsibs->rightSib_ != nullptr) {
        xsibs = xsibs->rightSib_.get();
    }

    return xsibs;
}

unsigned int Node::childCount() const
{
    Node* xsibs = child();
    unsigned int count = 0;

    while (xsibs != nullptr) {
        xsibs = xsibs->rightSib_.get();
        ++count;
    }

    return count;
}

Node* Node::next() const
{
    return rightSib_.get();
}

void Node::print(std::ostream* s) const
{
    *s << name();

    auto ptr = child();

    if (ptr != nullptr) {
        *s << '{';

        while (ptr != nullptr) {
            ptr->print(s);
            ptr = ptr->next();
            if (ptr != nullptr) {
                *s << ' ';
            }
        }

        *s << '}';
    }
}

void Node::graphviz(std::ostream& s) const
{
    s << "digraph ast {" << std::endl;
    s << R"(    forcelabels=true)" << std::endl;

    s << "    " << reinterpret_cast<std::uintptr_t>(this) << R"( [label=")" << name() << "\"]" << std::endl;

    subnodeGraphviz(s);

    s << "}" << std::endl;
}

void Node::subnodeGraphviz(std::ostream& s) const
{
    const Node* xsibs = child();

    // print children recursively
    while (xsibs != nullptr) {
        s << "    " << reinterpret_cast<std::uintptr_t>(xsibs) << R"( [label=")" << xsibs->name();// << "\"]";

        if (xsibs->symbolTable()) {
            s << "*";
        }

        if (xsibs->symbolTableEntry()) {
            s << "\\n" << xsibs->symbolTableEntry()->name() << " : " << xsibs->symbolTableEntry()->type()->str();
        }

        if (xsibs->type()) {
            s << "\\n<" << xsibs->type()->str() << '>';
        }

        s << "\"]";

        if (xsibs->marked) {
            s << "[style=filled, fillcolor=red]";
        } else if (xsibs->isLeaf()) {
            s << "[style=filled, fillcolor=skyblue]";
        }

        s << std::endl << "    " << reinterpret_cast<std::uintptr_t>(this) << " -> " << reinterpret_cast<std::uintptr_t>(xsibs) << std::endl;
        xsibs->subnodeGraphviz(s);
        xsibs = xsibs->next();
    }
}

bool Node::isLeaf() const
{
    return false;
}

std::shared_ptr<semantic::SymbolTable>& Node::symbolTable()
{
    return symbolTable_;
}

std::shared_ptr<semantic::SymbolTableEntry>& Node::symbolTableEntry()
{
    return symbolTableEntry_;
}

semantic::VariableType* Node::type() const
{
    return type_.get();
}

void Node::setType(std::unique_ptr<semantic::VariableType> type)
{
    type_ = std::move(type);
}

std::shared_ptr<semantic::SymbolTable> Node::closestSymbolTable()
{
    Node* curNode = this;

    // find closest symbol table
    while (curNode != nullptr && !curNode->symbolTable()) {
        curNode = curNode->parent();
    }

    return curNode != nullptr ? curNode->symbolTable() : nullptr;
}

const std::shared_ptr<semantic::SymbolTableEntry>& Node::symbolTableEntry() const
{
    return symbolTableEntry_;
}

const std::shared_ptr<semantic::SymbolTable>& Node::symbolTable() const
{
    return symbolTable_;
}

bool Leaf::isLeaf() const
{
    return !leftmostChild_;
}

void Leaf::subnodeGraphviz(std::ostream& s) const
{
    const Node* xsibs = child();

    s << "    " << reinterpret_cast<std::uintptr_t>(token_.get()) << R"( [label=")" << token_->value << "\", style=dashed]" << std::endl;
    s << "    " << reinterpret_cast<std::uintptr_t>(this) << " -> " << reinterpret_cast<std::uintptr_t>(token_.get()) << "[style=dashed, arrowhead=none]" << std::endl;

    // print children recursively
    while (xsibs != nullptr) {
        s << "    " << reinterpret_cast<std::uintptr_t>(xsibs) << R"( [label=")" << xsibs->name();// << "\"]";

        if (xsibs->symbolTable()) {
            s << "*";
        }

        if (xsibs->symbolTableEntry()) {
            s << "\\n" << xsibs->symbolTableEntry()->name() << " : " << xsibs->symbolTableEntry()->type()->str();
        }

        if (xsibs->type()) {
            s << "\\n<" << xsibs->type()->str() << '>';
        }

        s << "\"]";

        if (xsibs->marked) {
            s << "[style=filled, fillcolor=red]";
        } else if (xsibs->isLeaf()) {
            s << "[style=filled, fillcolor=skyblue]";
        }

        s << std::endl;
        s << "    " << reinterpret_cast<std::uintptr_t>(this) << " -> " << reinterpret_cast<std::uintptr_t>(xsibs) << std::endl;
        xsibs->subnodeGraphviz(s);
        xsibs = xsibs->next();
    }
}

void Leaf::print(std::ostream* s) const
{
    Node::print(s);

    if (token_ != nullptr) {
        *s << '(' << token_->value << ')';
    }
}

#define AST(NAME) \
void NAME::accept(Visitor* visitor)                      \
{                                                        \
    if (visitor->order() == VisitorOrder::NONE)          \
        visitor->visit(this);                            \
    else {                                               \
        if (visitor->order() == VisitorOrder::PREORDER)  \
            visitor->visit(this);                        \
                                                         \
        Node* xsibs = child();                           \
                                                         \
        while (xsibs != nullptr) {                       \
            xsibs->accept(visitor);                      \
            xsibs = xsibs->next();                       \
        }                                                \
                                                         \
        if (visitor->order() == VisitorOrder::POSTORDER) \
            visitor->visit(this);                        \
    }                                                    \
}

#define AST_LEAF(NAME) \
void NAME::accept(Visitor* visitor)                      \
{                                                        \
    if (visitor->order() == VisitorOrder::NONE)          \
        visitor->visit(this);                            \
    else {                                               \
        if (visitor->order() == VisitorOrder::PREORDER)  \
            visitor->visit(this);                        \
                                                         \
        Node* xsibs = child();                           \
                                                         \
        while (xsibs != nullptr) {                       \
            xsibs->accept(visitor);                      \
            xsibs = xsibs->next();                       \
        }                                                \
                                                         \
        if (visitor->order() == VisitorOrder::POSTORDER) \
            visitor->visit(this);                        \
    }                                                    \
}

#include "ast_nodes.h"

#undef AST
#undef AST_LEAF

}}