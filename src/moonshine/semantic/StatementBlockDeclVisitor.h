#pragma once

#include "moonshine/semantic/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace semantic {

class StatementBlockDeclVisitor : public Visitor
{
public:
    inline VisitorOrder order() override
    {
        return VisitorOrder::PREORDER;
    }

    void visit(ast::prog* node) override;
    void visit(ast::statBlock* node) override;
    void visit(ast::varDecl* node) override;
    void visit(ast::forStat* node) override;
    void visit(ast::classDecl* node) override;
    void visit(ast::funcDecl* node) override;
    void visit(ast::funcDef* node) override;
    void visit(ast::fparamList* node) override;
    void visit(ast::var* node) override;
private:
    void statBlockToSymbolTable(SymbolTable& table, ast::statBlock* node) const;
    void nodeToVariableType(VariableType& type, const ast::Node* node) const;
};

}}
