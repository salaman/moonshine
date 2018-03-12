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
    void visit(const ast::assignStat* node) override;
    void visit(const ast::varDecl* node) override;
};

}}
