#include "moonshine/semantic/TypeCheckVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"
#include "moonshine/semantic/SemanticError.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void TypeCheckVisitor::visit(ast::prog* node)
{
    Visitor::visit(node);

    // check that all member functions have been defined
    // TODO: move to funcDecl
    for (auto it = node->symbolTable()->begin(); it != node->symbolTable()->end(); ++it) {
        // find classes in global table
        if (it->second->kind() == SymbolTableEntryKind::CLASS) {
            auto classTable = it->second->link();

            // find functions in class table
            for (auto entry = classTable->begin(); entry != classTable->end(); ++entry) {
                // ensure the function has a symbol table link
                if (entry->second->kind() == SymbolTableEntryKind::FUNCTION && !entry->second->link()) {
                    errors_->emplace_back(SemanticErrorType::UNDEFINED_FUNCTION, nullptr); // TODO
                }
            }
        }
    }
}

void TypeCheckVisitor::visit(ast::assignStat* node)
{
    Visitor::visit(node);

    // ensure that lhs and rhs share the same type
    // only report an error if the types aren't errors themselves to prevent error spam
    if (*node->child(0)->type() != *node->child(1)->type()
        && node->child(0)->type()->isNotError()
        && node->child(1)->type()->isNotError()) {
        errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_TYPE, node->token());
    }
}

void TypeCheckVisitor::visit(ast::returnStat* node)
{
    Visitor::visit(node);

    // find the symbol table for the function we're in
    auto table = node->closestSymbolTable().get();
    while (table && table->parentEntry()->kind() != SymbolTableEntryKind::FUNCTION) {
        table = table->parentEntry()->parentTable();
    }

    if (!table) {
        throw std::runtime_error("TypeCheckVisitor::visit(returnStat): No symbol table exists");
    }

    // get the function's type
    auto type = dynamic_cast<FunctionType*>(table->parentEntry()->type());

    //if (!type) {
    //    throw std::runtime_error("TypeCheckVisitor::visit(returnStat): Invalid function type");
    //}

    // check that the return type matches the function's expected return type
    // only emit an error if the child is not already an error
    if (type && *node->child()->type() != type->returnType && node->child()->type()->isNotError()) {
        errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_RETURN_TYPE, node->token());
    }
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
        // TODO: error
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
            errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_TYPE, node->token());
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
            errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_TYPE, node->token());
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

    // dimList: ensure all subscripts are int type
    for (auto n = node->child(); n != nullptr; n = n->next()) {
        if (n->type()->type != Type::INT) {
            errors_->emplace_back(SemanticErrorType::INVALID_SUBSCRIPT_TYPE, dynamic_cast<ast::id*>(node->parent()->child(0))->token()); // TODO
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

void TypeCheckVisitor::visit(ast::funcDef* node)
{
    Visitor::visit(node);

    // ensure the function has a return if it needs one, and doesn't have a return if it can't have one

    auto entry = node->symbolTableEntry();
    auto type = dynamic_cast<FunctionType*>(entry->type());
    auto idNode = dynamic_cast<ast::nul*>(node->child(2))
                  ? dynamic_cast<ast::id*>(node->child(1))
                  : dynamic_cast<ast::id*>(node->child(2));

    if (type != nullptr && type->returnType.isNotError() && !entry->hasReturn()) {
        // this function needs a return and does not have one
        errors_->emplace_back(SemanticErrorType::MISSING_RETURN, idNode->token());
    } else if (type == nullptr && entry->hasReturn()) {
        // this function has a return but can't have one
        errors_->emplace_back(SemanticErrorType::INVALID_RETURN, idNode->token()); // TODO: this error should probably be on the returnStat
    }
}

}}
