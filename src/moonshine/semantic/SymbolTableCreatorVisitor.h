#pragma once

#include "moonshine/semantic/Visitor.h"
#include "moonshine/syntax/Node.h"

namespace moonshine { namespace semantic {

class SymbolTableCreatorVisitor : public Visitor
{
public:
    void visit(ast::prog* node) override;
    void visit(ast::classDecl* node) override;
    void visit(ast::funcDecl* node) override;
    void visit(ast::statBlock* node) override;
    void visit(ast::assignStat* node) override;
    void visit(ast::varDecl* node) override;
private:
    void nodeToVariableType(VariableType& type, const ast::Node* node) const;
    void nodeToFunctionType(FunctionType& type, const ast::Node* node) const;
};

}}
