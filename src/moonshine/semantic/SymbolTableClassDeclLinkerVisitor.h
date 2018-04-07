#pragma once

#include "moonshine/semantic/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace semantic {

class SymbolTableClassDeclLinkerVisitor : public Visitor
{
public:
    void visit(ast::classDecl* node) override;
};

}}
