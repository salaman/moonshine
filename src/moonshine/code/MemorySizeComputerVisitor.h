#pragma once

#include "moonshine/Visitor.h"
#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/Type.h"

namespace moonshine { namespace code {

class MemorySizeComputerVisitor : public Visitor
{
public:
    void visit(ast::prog* node) override;
    void visit(ast::varDecl* node) override;
    void visit(ast::classDecl* node) override;
    void visit(ast::funcDef* node) override;
    void visit(ast::forStat* node) override;
    void visit(ast::addOp* node) override;
    void visit(ast::multOp* node) override;
    void visit(ast::relOp* node) override;
    void visit(ast::num* node) override;
    void visit(ast::var* node) override;
    void visit(ast::dataMember* node) override;
    void visit(ast::fCall* node) override;
private:
    int getPrimitiveSize(const semantic::Type& type);
};

}}
