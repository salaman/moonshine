#pragma once

#include "moonshine/semantic/Visitor.h"
#include "moonshine/syntax/Node.h"

#include <memory>
#include <utility>
#include <type_traits>
#include <iostream>

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
};

}}
