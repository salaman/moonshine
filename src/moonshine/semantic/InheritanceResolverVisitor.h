#pragma once

#include "moonshine/semantic/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace semantic {

class InheritanceResolverVisitor : public Visitor
{
public:
    void visit(ast::classList* node) override;
    void visit(ast::inherList* node) override;
    void visit(ast::membList* node) override;
private:
};

}}
