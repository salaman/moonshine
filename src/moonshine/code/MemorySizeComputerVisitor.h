#pragma once

#include "moonshine/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace code {

class MemorySizeComputerVisitor : public Visitor
{
public:
    void visit(ast::prog* node) override;
};

}}
