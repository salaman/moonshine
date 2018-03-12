#include "moonshine/semantic/TypeCheckVisitor.h"

#include <iostream>

namespace moonshine { namespace semantic {

void TypeCheckVisitor::visit(const ast::assignStat* node)
{
    Visitor::visit(node);

    std::cout << "TypeCheckVisitor::visit(assignStat)" << std::endl;
}

void TypeCheckVisitor::visit(const ast::varDecl* node)
{
    Visitor::visit(node);

    std::cout << "TypeCheckVisitor::visit(varDecl)" << std::endl;
}

}}
