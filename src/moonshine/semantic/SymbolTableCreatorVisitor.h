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
    void visit(ast::funcDef* node) override;
    void visit(ast::statBlock* node) override;
    void visit(ast::varDecl* node) override;
    void visit(ast::fparam* node) override;
private:
    void statBlockToSymbolTable(SymbolTable& table, ast::statBlock* node) const;
    void fparamListToSymbolTable(SymbolTable& table, ast::fparamList* node) const;
    void nodeToVariableType(VariableType& type, const ast::Node* node) const;
    void funcDeclToFunctionType(FunctionType& type, const ast::Node* node) const;
    void funcDefToFunctionType(FunctionType& type, const ast::Node* node) const;
};

}}
