#pragma once

#include "moonshine/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace semantic {

class InheritanceResolverVisitor : public Visitor
{
public:
    void visit(ast::classList* node) override;
    void visit(ast::classDecl* node) override;
    void visit(ast::inherList* node) override;
    void visit(ast::membList* node) override;
private:
};

}}
