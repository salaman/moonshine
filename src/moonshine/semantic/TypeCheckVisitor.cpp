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

    if (*node->child(0)->type() != *node->child(1)->type()) {
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

    if (*node->child(0)->type() != *node->child(1)->type()) {
        errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_TYPE);
    }
}

void TypeCheckVisitor::visit(ast::addOp* node)
{
    Visitor::visit(node);

    if (*node->child(0)->type() == *node->child(1)->type()) {
        node->setType(std::unique_ptr<VariableType>(new VariableType(*node->child(0)->type())));
    } else {
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        node->setType(std::move(type));
    }
}

void TypeCheckVisitor::visit(ast::type* node)
{
    Visitor::visit(node);
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

void TypeCheckVisitor::visit(ast::var* node)
{
    Visitor::visit(node);

    // a var's type is its rightmost data member's type
    ast::Node* rightmostChild = node->rightmostChild();

    node->setType(std::unique_ptr<VariableType>(new VariableType(*rightmostChild->type())));
}

void TypeCheckVisitor::visit(ast::dataMember* node)
{
    Visitor::visit(node);

    // find closest symbol table
    std::shared_ptr<SymbolTable> table = node->symbolTable();
    ast::Node* curNode = node;

    while (!table && curNode) {
        curNode = curNode->parent();
        table = curNode->symbolTable();
    }

    if (!table) {
        throw std::runtime_error("TypeCheckVisitor::visit(id): No symbol table exists");
    }

    auto idNode = dynamic_cast<ast::id*>(node->child(0));

    // locate the entry for this id in the symtab
    auto entry = (*table)[idNode->token()->value];
    VariableType* p;

    if (entry && (p = dynamic_cast<VariableType*>(entry->type()))) {
        // if we find it, set our type to a copy of its type
        //std::unique_ptr<VariableType> type(new VariableType(*p));
        std::unique_ptr<VariableType> type(new VariableType());

        auto indiceCount = node->child(1)->childCount();

        // TODO: check that each index <= dimsize
        if (p->indices.size() != indiceCount) {
            type->type = Type::ERROR;
            errors_->emplace_back(SemanticErrorType::INVALID_DIMENSION_COUNT);
        } else {
            type->type = p->type;
        }

        node->setType(std::move(type));
    } else {
        // else, this is an error
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        node->setType(std::move(type));
    }
}

void TypeCheckVisitor::visit(ast::fCall* node)
{
    Visitor::visit(node);

    // TODO
    std::unique_ptr<VariableType> type(new VariableType());
    type->type = Type::ERROR;
    node->setType(std::move(type));
}

}}
