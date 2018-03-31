#pragma once

#include "moonshine/lexer/Token.h"
#include "moonshine/semantic/SymbolTable.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <ostream>

namespace moonshine {

class Visitor;

namespace ast {

class Node
{
public:
    virtual inline const char* name() const { return "Node"; };

    static std::unique_ptr<Node> makeNode(const std::string& name, std::shared_ptr<Token>& op);

    void makeSiblings(std::unique_ptr<Node> y);
    void adoptChildren(std::unique_ptr<Node> y);

    Node* parent() const;
    Node* child() const;
    Node* child(const unsigned int& index) const;
    Node* rightmostChild() const;
    unsigned int childCount() const;
    Node* next() const;
    virtual bool isLeaf() const;

    virtual void accept(Visitor* visitor) = 0; // TODO: make const param
    std::shared_ptr<semantic::SymbolTable>& symbolTable();
    std::shared_ptr<semantic::SymbolTable> closestSymbolTable();
    std::shared_ptr<semantic::SymbolTableEntry>& symbolTableEntry();
    semantic::VariableType* type() const;
    void setType(std::unique_ptr<semantic::VariableType> type);

    virtual void print(std::ostream* s) const;
    void graphviz(std::ostream& s) const;
    virtual void subnodeGraphviz(std::ostream& s) const;

    bool marked = false;
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

    // type checking
    std::unique_ptr<semantic::VariableType> type_ = nullptr;
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

    void accept(Visitor* visitor) override = 0;

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
    void accept(Visitor* visitor) override; \
};

#define AST(NAME)                                                            \
class NAME : public Node                                                     \
{                                                                            \
public:                                                                      \
    inline const char* name() const override { return #NAME; };              \
    void accept(Visitor* visitor) override; \
};

#include "ast_nodes.h"

#undef AST
#undef AST_LEAF

}}