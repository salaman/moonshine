#pragma once

#include "moonshine/lexer/Token.h"

#include <memory>
#include <ostream>

namespace moonshine { namespace ast {

class Node
{
public:
    virtual inline const char* name() const
    { return "Node"; };

    static std::unique_ptr<Node> makeNode(const std::string& name, std::shared_ptr<Token>& op);

    template<typename... Args>
    static void makeFamily(std::unique_ptr<Node> op, Args... args);

    void makeSiblings(std::unique_ptr<Node> y);
    void adoptChildren(std::unique_ptr<Node> y);

    Node* child() const;
    Node* next() const;
    virtual bool isLeaf() const;
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

    bool isLeaf() const override ;
    void print(std::ostream* s) const override;
    void subnodeGraphviz(std::ostream& s) const override;
protected:
    const std::shared_ptr<Token> token_;
};

#define AST_LEAF(NAME) \
class NAME : public Leaf \
{ \
public: \
    explicit NAME(std::shared_ptr<Token>& token) : Leaf(token) {} \
    inline const char* name() const override { return #NAME; }; \
}

#define AST(NAME) \
class NAME : public Node \
{ \
public: \
    inline const char* name() const override { return #NAME; }; \
}

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
AST(funcDecl);

#undef AST
#undef AST_LEAF

}}