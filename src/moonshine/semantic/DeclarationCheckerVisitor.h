#pragma once

#include "moonshine/semantic/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace semantic {

class DeclarationCheckerVisitor : public Visitor
{
public:
    inline VisitorOrder order() override
    {
        return VisitorOrder::PREORDER;
    }

    void visit(ast::forStat* node) override;
    void visit(ast::varDecl* node) override;
    void visit(ast::dataMember* node) override;
    void visit(ast::fCall* node) override;
private:
};

}}
