#pragma once

#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/SemanticError.h"

#include <vector>

namespace moonshine { namespace semantic {

class Visitor
{
public:
    #define AST(NAME) virtual void visit(ast::NAME* node) {}
    #define AST_LEAF(NAME) virtual void visit(ast::NAME* node) {}

    #include "moonshine/syntax/ast_nodes.h"

    #undef AST
    #undef AST_LEAF

    void setErrorContainer(std::vector<SemanticError>* errors);

protected:
    std::vector<SemanticError>* errors_ = nullptr;
};

}}
