#pragma once

#include "moonshine/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace semantic {

class SymbolTableLinkerVisitor : public Visitor
{
public:
    inline VisitorOrder order() override
    {
        // this is a preorder traversal visitor because of forStat scopes and varDecls
        return VisitorOrder::PREORDER;
    }

    void visit(ast::varDecl* node) override;
    void visit(ast::forStat* node) override;
    void visit(ast::funcDecl* node) override;
    void visit(ast::funcDef* node) override;
    void visit(ast::returnStat* node) override;
private:
    void nodeToVariableType(VariableType& type, const ast::Node* node) const;
    void fparamListToSymbolTable(SymbolTable& table, ast::fparamList* node) const;
};

}}
