#pragma once

#include "moonshine/lexer/Token.h"
#include "moonshine/semantic/SymbolTable.h"

#include <memory>
#include <ostream>

namespace moonshine {

namespace semantic {
    class Visitor;
}

namespace ast {

class Node
{
public:
    virtual inline const char* name() const { return "Node"; };

    static std::unique_ptr<Node> makeNode(const std::string& name, std::shared_ptr<Token>& op);

    template<typename... Args>
    static void makeFamily(std::unique_ptr<Node> op, Args... args);

    void makeSiblings(std::unique_ptr<Node> y);
    void adoptChildren(std::unique_ptr<Node> y);

    Node* child() const;
    Node* child(const unsigned int& index) const;
    Node* next() const;
    virtual bool isLeaf() const;

    virtual void accept(const std::shared_ptr<semantic::Visitor>& visitor) = 0;
    std::shared_ptr<semantic::SymbolTable>& symbolTable();
    std::shared_ptr<semantic::SymbolTableEntry>& symbolTableEntry();

    virtual void print(std::ostream* s) const;
    void graphviz(std::ostream& s) const;
    virtual void subnodeGraphviz(std::ostream& s) const;
protected:
    // parent_
    Node* parent_ = nullptr;

    // siblings
    std::unique_ptr<Node> rightSib_ = nullptr;
    Node* leftmostSib_ = nullptr;

    // children
    std::unique_ptr<Node> leftmostChild_ = nullptr;

    // symbol table entries
    std::shared_ptr<semantic::SymbolTable> symbolTable_;
    std::shared_ptr<semantic::SymbolTableEntry> symbolTableEntry_;
};

class Leaf : public Node
{
public:
    Leaf(std::shared_ptr<Token>& token) : token_(token)
    {}

    inline const std::shared_ptr<Token>& token() const
    {
        return token_;
    }

    bool isLeaf() const override;

    void accept(const std::shared_ptr<semantic::Visitor>& visitor) override = 0;

    void print(std::ostream* s) const override;
    void subnodeGraphviz(std::ostream& s) const override;
protected:
    const std::shared_ptr<Token> token_;
};

#define AST_LEAF(NAME)                                                       \
class NAME : public Leaf                                                     \
{                                                                            \
public:                                                                      \
    explicit NAME(std::shared_ptr<Token>& token) : Leaf(token) {}            \
    inline const char* name() const override { return #NAME; };              \
    void accept(const std::shared_ptr<semantic::Visitor>& visitor) override; \
};

#define AST(NAME)                                                            \
class NAME : public Node                                                     \
{                                                                            \
public:                                                                      \
    inline const char* name() const override { return #NAME; };              \
    void accept(const std::shared_ptr<semantic::Visitor>& visitor) override; \
};

#include "ast_nodes.h"

#undef AST
#undef AST_LEAF

}}