#include "moonshine/semantic/TypeCheckVisitor.h"

#include <iostream>

namespace moonshine { namespace semantic {

void TypeCheckVisitor::visit(ast::prog* node)
{
    Visitor::visit(node);

    std::cout << "TypeCheckVisitor::visit(prog)" << std::endl;
}

void TypeCheckVisitor::visit(ast::assignStat* node)
{
    Visitor::visit(node);

    std::cout << "TypeCheckVisitor::visit(assignStat)" << std::endl;
}

void TypeCheckVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);

    std::cout << "TypeCheckVisitor::visit(varDecl)" << std::endl;
}

void TypeCheckVisitor::visit(ast::num* node)
{
    Visitor::visit(node);

    std::cout << "TypeCheckVisitor::visit(num) " << node->token()->value << std::endl;
}

}}
