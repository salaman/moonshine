#pragma once

#include "moonshine/lexer/Token.h"

#include <memory>
#include <iostream>

namespace moonshine { namespace ast {

class Node
{
public:
    virtual inline const char* name() const
    { return "Node"; };

    static std::unique_ptr<Node> makeNode(const std::string& name, Token* op);

    template<typename... Args>
    static void makeFamily(std::unique_ptr<Node> op, Args... args);

    void makeSiblings(std::unique_ptr<Node> y);
    void adoptChildren(std::unique_ptr<Node> y);

    Node* child() const;
    Node* next() const;

    virtual void print() const;
protected:
    // parent_
    Node* parent_ = nullptr;

    // siblings
    std::unique_ptr<Node> rightSib_ = nullptr;
    Node* leftmostSib_ = nullptr;

    // children
    std::unique_ptr<Node> leftmostChild_ = nullptr;
};

class Leaf : public Node
{
public:
    Leaf(const Token* token) : token_(token)
    {}

    inline const Token* token() const
    {
        return token_;
    }

    void print() const override;
protected:
    const Token* token_ = nullptr;
};

#define AST_LEAF(NAME) \
    class NAME : public Leaf \
    { \
    public: \
        explicit NAME(const Token* token) : Leaf(token) {} \
        inline const char* name() const override { return #NAME; }; \
    }

#define AST(NAME) \
    class NAME : public Node \
    { \
    public: \
        inline const char* name() const override { return #NAME; }; \
    }

AST_LEAF(RelOp);

AST_LEAF(AddOp);

AST_LEAF(MultOp);

AST_LEAF(Num);

AST_LEAF(Id);

AST_LEAF(Type);

AST_LEAF(Sign);

AST(GetStat);

AST(PutStat);

AST(ReturnStat);

AST(ForStat);

AST(Not);

AST(IfStat);

AST(AssignStat);

AST(StatBlock);

}}