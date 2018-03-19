#include "moonshine/semantic/TypeCheckVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"
#include "moonshine/semantic/SemanticError.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void TypeCheckVisitor::visit(ast::assignStat* node)
{
    Visitor::visit(node);

    // ensure that lhs and rhs share the same type
    // only report an error if the types aren't errors themselves to prevent error spam
    if (*node->child(0)->type() != *node->child(1)->type()
        && node->child(0)->type()->isNotError()
        && node->child(1)->type()->isNotError()) {
        errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_TYPE);
    }
}

void TypeCheckVisitor::visit(ast::returnStat* node)
{
    Visitor::visit(node);
}

void TypeCheckVisitor::visit(ast::relOp* node)
{
    Visitor::visit(node);

    // if lhs and rhs are not of the same type, error
    // only report an error if the types aren't errors themselves to prevent error spam
    if (*node->child(0)->type() != *node->child(1)->type()
        && node->child(0)->type()->isNotError()
        && node->child(1)->type()->isNotError()) {
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        node->setType(std::move(type));
    }
}

void TypeCheckVisitor::visit(ast::addOp* node)
{
    Visitor::visit(node);

    if (*node->child(0)->type() == *node->child(1)->type()) {
        // if lhs and rhs are of the same type, set the op's type to the same one
        node->setType(std::unique_ptr<VariableType>(new VariableType(*node->child(0)->type())));
    } else {
        // else, set it to error
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        node->setType(std::move(type));

        // only report an error if the types aren't errors themselves to prevent error spam
        if (node->child(0)->type()->isNotError() && node->child(1)->type()->isNotError()) {
            errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_TYPE);
        }
    }
}

void TypeCheckVisitor::visit(ast::multOp* node)
{
    Visitor::visit(node);

    if (*node->child(0)->type() == *node->child(1)->type()) {
        // if lhs and rhs are of the same type, set the op's type to the same one
        node->setType(std::unique_ptr<VariableType>(new VariableType(*node->child(0)->type())));
    } else {
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        node->setType(std::move(type));

        if (!node->child(0)->type() || !node->child(1)->type()) {
            node->marked = true;
        } else

        // only report an error if the types aren't errors themselves to prevent error spam
        if (node->child(0)->type()->isNotError() && node->child(1)->type()->isNotError()) {
            errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_TYPE);
        }
    }
}

void TypeCheckVisitor::visit(ast::num* node)
{
    Visitor::visit(node);

    std::unique_ptr<VariableType> type(new VariableType());

    // type
    switch (node->token()->type) {
        case TokenType::T_INTEGER_LITERAL:
            type->type = Type::INT;
            break;
        case TokenType::T_FLOAT_LITERAL:
            type->type = Type::FLOAT;
            break;
        default:
            throw std::runtime_error("TypeCheckVisitor::visit(num): Invalid AST num node");
    }

    node->setType(std::move(type));
}

void TypeCheckVisitor::visit(ast::indexList* node)
{
    Visitor::visit(node);

    // dimList
    for (auto n = node->child(); n != nullptr; n = n->next()) {
        if (n->type()->type != Type::INT) {
            errors_->emplace_back(SemanticErrorType::INVALID_SUBSCRIPT_TYPE);
        }
    }
}

void TypeCheckVisitor::visit(ast::notFactor* node)
{
    Visitor::visit(node);

    // bubble up child type
    std::unique_ptr<VariableType> type(new VariableType(*node->child()->type()));
    node->setType(std::move(type));
}

void TypeCheckVisitor::visit(ast::sign* node)
{
    Visitor::visit(node);

    // bubble up child type
    std::unique_ptr<VariableType> type(new VariableType(*node->child()->type()));
    node->setType(std::move(type));
}

}}
