#pragma once

#include "moonshine/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace semantic {

class ShadowedSymbolCheckerVisitor : public Visitor
{
public:
    void visit(ast::forStat* node) override;
    void visit(ast::varDecl* node) override;
    void visit(ast::membList* node) override;
};

}}
