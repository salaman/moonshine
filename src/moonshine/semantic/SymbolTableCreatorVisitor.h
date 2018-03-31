#pragma once

#include "moonshine/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace semantic {

class SymbolTableCreatorVisitor : public Visitor
{
public:
    void visit(ast::prog* node) override;
    void visit(ast::classDecl* node) override;
    void visit(ast::funcDecl* node) override;
    void visit(ast::funcDef* node) override;
    void visit(ast::varDecl* node) override;
    void visit(ast::fparam* node) override;
    void visit(ast::forStat* node) override;
private:
    void fparamListToSymbolTable(SymbolTable& table, ast::fparamList* node) const;
    void nodeToVariableType(VariableType& type, const ast::Node* node) const;
    void funcDeclToFunctionType(FunctionType& type, const ast::Node* node) const;
    void funcDefToFunctionType(FunctionType& type, const ast::Node* node) const;
};

}}
