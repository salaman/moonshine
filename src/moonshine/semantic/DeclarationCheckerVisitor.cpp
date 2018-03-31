#include "moonshine/semantic/DeclarationCheckerVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void DeclarationCheckerVisitor::visit(ast::dataMember* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("DeclarationCheckerVisitor::visit(dataMember): No symbol table exists");
    }

    auto varNode = node->parent();
    auto idNode = dynamic_cast<ast::id*>(node->child(0));
    auto indexListNode = dynamic_cast<ast::indexList*>(node->child(1));

    if (!varNode->type()) {
        // this is the first dataMember under var

        // check if this symbol has been previously declared in this scope
        if (auto entry = (*table)[idNode->token()->value]) {

            // find the var's type after this dataMember is applied
            if (auto t = dynamic_cast<VariableType*>(entry->type())) {
                // if we find it, set our type to a copy of its type
                std::unique_ptr<VariableType> type(new VariableType());
                auto indiceCount = indexListNode->childCount();

                // check that the number of indices matches the dimension of the original type
                if (t->indices.size() != indiceCount) {
                    // if not, this is an error
                    type->type = Type::ERROR;
                    errors_->emplace_back(SemanticErrorType::INVALID_DIMENSION_COUNT, idNode->token());
                } else {
                    type->type = t->type;
                    type->className = t->className;
                }

                //node->setType(std::unique_ptr<VariableType>(new VariableType(*type)));
                varNode->setType(std::move(type));
            } else {
                // the symbol exists, but is not a variable
                std::unique_ptr<VariableType> type(new VariableType());
                type->type = Type::ERROR;
                varNode->setType(std::move(type));
                errors_->emplace_back(SemanticErrorType::INVALID_VARIABLE, idNode->token());
            }

        } else {
            // the id was not found in the symbol table
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::UNDECLARED_VARIABLE, idNode->token());
        }

    } else if (varNode->type()->type == Type::CLASS) {
        // this is NOT the first dataMember under var
        // therefore, mutate var's type based on this dataMember

        // get the class of the member
        auto entry = (*table)[varNode->type()->className];
        auto member = (*entry->link())[idNode->token()->value];

        if (!member) {
            // the class is valid, but the member doesn't exist
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::UNDECLARED_MEMBER_VARIABLE, idNode->token());
        } else if (auto t = dynamic_cast<VariableType*>(member->type())) {
            // the member exists and is a variable
            // set our type to a copy of its type
            std::unique_ptr<VariableType> type(new VariableType());
            auto indiceCount = indexListNode->childCount();

            // check that the number of indices matches the dimension of the original type
            if (t->indices.size() != indiceCount) {
                // if not, this is an error
                type->type = Type::ERROR;
                errors_->emplace_back(SemanticErrorType::INVALID_DIMENSION_COUNT, idNode->token());
            } else {
                type->type = t->type;
                type->className = t->className;
            }

            varNode->setType(std::move(type));
        } else {
            // the member exists, but is not a variable (ie. it is a function)
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::INVALID_VARIABLE, idNode->token());
        }

    } else if (varNode->type()->type == Type::ERROR) {
        // don't process errors
    } else {
        // an int or float terminates the chain, therefore this data member is invalid
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        varNode->setType(std::move(type));
        errors_->emplace_back(SemanticErrorType::INVALID_VARIABLE, idNode->token());
    }
}

void DeclarationCheckerVisitor::visit(ast::fCall* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("DeclarationCheckerVisitor::visit(fCall): No symbol table exists");
    }

    auto varNode = node->parent();
    auto idNode = dynamic_cast<ast::id*>(node->child(0));

    // TODO: aParams visitor to flatten type and compare it easier

    if (!varNode->type()) {
        // this is the first dataMember under var

        // check if this symbol has been previously declared in this scope (it should be a free function)
        if (auto entry = (*table)[idNode->token()->value]) {

            // find the var's type after this fCall is applied
            if (auto t = dynamic_cast<FunctionType*>(entry->type())) {
                // if we find it, set our type to the function's return type
                std::unique_ptr<VariableType> type(new VariableType(t->returnType));
                node->setType(std::unique_ptr<VariableType>(new VariableType(*type)));
                varNode->setType(std::move(type));
            } else {
                // the symbol exists, but is not a function
                std::unique_ptr<VariableType> type(new VariableType());
                type->type = Type::ERROR;
                varNode->setType(std::move(type));
                errors_->emplace_back(SemanticErrorType::INVALID_FUNCTION, idNode->token());
            }

        } else {
            // the id was not found in the symbol table
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::UNDECLARED_FUNCTION, idNode->token());
        }

    } else if (varNode->type()->type == Type::CLASS) {
        // this is NOT the first dataMember under var
        // therefore, mutate var's type based on this dataMember

        // get the class of the member
        auto entry = (*table)[varNode->type()->className];
        auto member = (*entry->link())[idNode->token()->value];

        if (!member) {
            // the class is valid, but the function doesn't exist in the class
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::UNDECLARED_MEMBER_FUNCTION, idNode->token());
        } else if (auto t = dynamic_cast<FunctionType*>(member->type())) {
            // the member exists and is a function
            // set our type to its return type
            std::unique_ptr<VariableType> type(new VariableType(t->returnType));
            varNode->setType(std::move(type));
        } else {
            // the member exists, but is not a function (ie. it is a variable)
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::INVALID_FUNCTION, idNode->token());
        }

    } else if (varNode->type()->type == Type::ERROR) {
        // don't process errors
    } else {
        // an int or float terminates the chain, therefore this data member is invalid
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        varNode->setType(std::move(type));
        errors_->emplace_back(SemanticErrorType::INVALID_FUNCTION, idNode->token());
    }
}

void DeclarationCheckerVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);

    auto entry = node->symbolTableEntry();

    // ignore any varDecl nodes that aren't being used (multiply declared, etc.)
    if (!entry || !entry->parentTable()) {
        return;
    }

    // get the local table and the parent table
    auto table = entry->parentTable();
    auto parentTable = table->parentEntry()->parentTable();

    // shadow-check this varDecl by seeing if it exists in this table and not the parent table
    if ((*parentTable)[entry->name()]) {
        errors_->emplace_back(SemanticErrorType::SHADOWED_VARIABLE, dynamic_cast<ast::id*>(node->child(1))->token(), SemanticErrorLevel::WARN);
        node->marked = true;
    }
}

void DeclarationCheckerVisitor::visit(ast::forStat* node)
{
    Visitor::visit(node);

    auto entry = node->symbolTableEntry();

    // ignore any forStat nodes that aren't being used
    if (!entry || !entry->parentTable()) {
        return;
    }

    // get the local table and the parent table
    auto table = entry->parentTable();
    auto parentTable = table->parentEntry()->parentTable();

    // shadow-check this varDecl by seeing if it exists in this table and not the parent table
    auto id = dynamic_cast<ast::id*>(node->child(1));
    if ((*parentTable)[id->token()->value]) {
        errors_->emplace_back(SemanticErrorType::SHADOWED_VARIABLE, id->token(), SemanticErrorLevel::WARN);
        node->marked = true;
    }
}

}}
