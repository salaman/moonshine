#pragma once

#include "moonshine/syntax/Node.h"
#include "moonshine/semantic/SemanticError.h"

#include <vector>

namespace moonshine {

enum class VisitorOrder
{
    NONE,
    POSTORDER,
    PREORDER,
};

class Visitor
{
public:
    inline virtual VisitorOrder order()
    {
        return VisitorOrder::POSTORDER;
    }

    #define AST(NAME) virtual void visit(ast::NAME* node) { if (order() == VisitorOrder::NONE) next(node); }
    #define AST_LEAF(NAME) virtual void visit(ast::NAME* node) { if (order() == VisitorOrder::NONE) next(node); }

    #include "moonshine/syntax/ast_nodes.h"

    #undef AST
    #undef AST_LEAF

    void setErrorContainer(std::vector<semantic::SemanticError>* errors);

protected:
    std::vector<semantic::SemanticError>* errors_ = nullptr;

    void next(ast::Node* node);
};

}
