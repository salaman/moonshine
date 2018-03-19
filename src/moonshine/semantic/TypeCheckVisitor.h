#pragma once

#include "moonshine/semantic/Visitor.h"
#include "moonshine/syntax/Node.h"

#include <memory>
#include <utility>
#include <type_traits>
#include <iostream>

namespace moonshine { namespace semantic {

class TypeCheckVisitor : public Visitor
{
public:
    void visit(ast::num* node) override;
    void visit(ast::addOp* node) override;
    void visit(ast::multOp* node) override;
    void visit(ast::relOp* node) override;
    void visit(ast::notFactor* node) override;
    void visit(ast::sign* node) override;
    void visit(ast::assignStat* node) override;
    void visit(ast::returnStat* node) override;
    void visit(ast::indexList* node) override;
};

}}
