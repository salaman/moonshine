#pragma once

#include "moonshine/syntax/Node.h"

#include <memory>
#include <utility>
#include <type_traits>
#include <iostream>

namespace moonshine { namespace semantic {

class Visitor
{
public:
    #define AST(NAME) virtual void visit(const ast::NAME* node) {}

    #define AST_LEAF(NAME) virtual void visit(const ast::NAME* node) {}

    #include "moonshine/syntax/ast_nodes.h"

    #undef AST
    #undef AST_LEAF
};

}}
