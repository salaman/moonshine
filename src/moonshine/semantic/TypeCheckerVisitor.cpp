#include "moonshine/semantic/TypeCheckerVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"
#include "moonshine/semantic/SemanticError.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void TypeCheckerVisitor::visit(ast::assignStat* node)
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

void TypeCheckerVisitor::visit(ast::returnStat* node)
{
    Visitor::visit(node);

    // find the symbol table for the function we're in
    auto table = node->closestSymbolTable().get();
    while (table && table->parentEntry()->kind() != SymbolTableEntryKind::FUNCTION) {
        table = table->parentEntry()->parentTable();
    }

    if (!table) {
        throw std::runtime_error("TypeCheckerVisitor::visit(returnStat): No symbol table exists");
    }

    // get the function's type
    auto type = dynamic_cast<FunctionType*>(table->parentEntry()->type());

    //if (!type) {
    //    throw std::runtime_error("TypeCheckerVisitor::visit(returnStat): Invalid function type");
    //}

    // check that the return type matches the function's expected return type
    // only emit an error if the child is not already an error
    if (type && *node->child()->type() != type->returnType && node->child()->type()->isNotError()) {
        errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_RETURN_TYPE, node->token());
    }
}

void TypeCheckerVisitor::visit(ast::relOp* node)
{
    Visitor::visit(node);

    // if lhs and rhs are not of the same type, error
    if (*node->child(0)->type() == *node->child(1)->type()) {
        auto table = node->closestSymbolTable();
        node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
        node->symbolTableEntry()->setName(nextTempVar());
        node->symbolTableEntry()->setKind(SymbolTableEntryKind::TEMPVAR);
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::INT;
        node->setType(std::unique_ptr<VariableType>(new VariableType(*type)));
        node->symbolTableEntry()->setType(std::move(type));
        table->addEntry(node->symbolTableEntry());
    } else {
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        node->setType(std::move(type));

        // only report an error if the types aren't errors themselves to prevent error spam
        if (node->child(0)->type()->isNotError() && node->child(1)->type()->isNotError()) {
            errors_->emplace_back(SemanticErrorType::INCOMPATIBLE_TYPE, node->token());
        }
    }
}

void TypeCheckerVisitor::visit(ast::addOp* node)
{
    Visitor::visit(node);

    if (*node->child(0)->type() == *node->child(1)->type()) {
        // if lhs and rhs are of the same type, set the op's type to the same one
        node->setType(std::unique_ptr<VariableType>(new VariableType(*node->child(0)->type())));

        auto table = node->closestSymbolTable();
        node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
        node->symbolTableEntry()->setName(nextTempVar());
        node->symbolTableEntry()->setKind(SymbolTableEntryKind::TEMPVAR);
        node->symbolTableEntry()->setType(std::unique_ptr<VariableType>(new VariableType(*node->type())));
        table->addEntry(node->symbolTableEntry());
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

void TypeCheckerVisitor::visit(ast::multOp* node)
{
    Visitor::visit(node);

    if (*node->child(0)->type() == *node->child(1)->type()) {
        // if lhs and rhs are of the same type, set the op's type to the same one
        node->setType(std::unique_ptr<VariableType>(new VariableType(*node->child(0)->type())));

        auto table = node->closestSymbolTable();
        node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
        node->symbolTableEntry()->setName(nextTempVar());
        node->symbolTableEntry()->setKind(SymbolTableEntryKind::TEMPVAR);
        node->symbolTableEntry()->setType(std::unique_ptr<VariableType>(new VariableType(*node->type())));
        table->addEntry(node->symbolTableEntry());
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

void TypeCheckerVisitor::visit(ast::num* node)
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
            throw std::runtime_error("TypeCheckerVisitor::visit(num): Invalid AST num node");
    }

    node->setType(std::move(type));

    // for literals, we create a tempvar
    if (!dynamic_cast<ast::dimList*>(node->parent())) {
        auto table = node->closestSymbolTable();
        node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
        node->symbolTableEntry()->setName(nextTempVar());
        node->symbolTableEntry()->setKind(SymbolTableEntryKind::LITERAL);
        node->symbolTableEntry()->setType(std::unique_ptr<VariableType>(new VariableType(*node->type())));
        table->addEntry(node->symbolTableEntry());
    }
}

void TypeCheckerVisitor::visit(ast::indexList* node)
{
    Visitor::visit(node);

    // dimList: ensure all subscripts are int type
    for (auto n = node->child(); n != nullptr; n = n->next()) {
        if (n->type()->type != Type::INT) {
            errors_->emplace_back(SemanticErrorType::INVALID_SUBSCRIPT_TYPE, dynamic_cast<ast::id*>(node->parent()->child(0))->token()); // TODO
        }
    }
}

void TypeCheckerVisitor::visit(ast::notFactor* node)
{
    Visitor::visit(node);

    // bubble up child type
    //std::unique_ptr<VariableType> type(new VariableType(*node->child()->type()));
    //node->setType(std::move(type));

    auto table = node->closestSymbolTable();
    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(nextTempVar());
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::TEMPVAR);
    std::unique_ptr<VariableType> type(new VariableType());
    type->type = Type::INT;
    node->setType(std::unique_ptr<VariableType>(new VariableType(*type)));
    node->symbolTableEntry()->setType(std::move(type));
    table->addEntry(node->symbolTableEntry());
}

void TypeCheckerVisitor::visit(ast::sign* node)
{
    Visitor::visit(node);

    // TODO: sign can only be used on numerics

    // bubble up child type
    std::unique_ptr<VariableType> type(new VariableType(*node->child()->type()));
    node->setType(std::unique_ptr<VariableType>(new VariableType(*type)));

    auto table = node->closestSymbolTable();
    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(nextTempVar());
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::TEMPVAR);
    node->symbolTableEntry()->setType(std::move(type));
    table->addEntry(node->symbolTableEntry());
}

void TypeCheckerVisitor::visit(ast::funcDecl* node)
{
    Visitor::visit(node);

    auto entry = node->symbolTableEntry();

    if (!entry) {
        return;
    }

    // check that all member functions have been defined
    if (entry->kind() == SymbolTableEntryKind::FUNCTION && !entry->link()) {
        errors_->emplace_back(SemanticErrorType::UNDEFINED_FUNCTION, dynamic_cast<ast::id*>(node->child(1))->token());
    }
}

void TypeCheckerVisitor::visit(ast::funcDef* node)
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

    // create an entry for the this pointer
    auto table = node->symbolTable();
    auto thisVar = std::make_shared<SymbolTableEntry>();
    thisVar->setName("_this");
    thisVar->setKind(SymbolTableEntryKind::THIS);
    std::unique_ptr<VariableType> thisType(new VariableType());
    thisType->type = Type::INT;
    thisVar->setType(std::move(thisType));
    thisVar->setSize(4);
    table->addEntry(thisVar);
}

void TypeCheckerVisitor::visit(ast::dataMember* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("TypeCheckerVisitor::visit(dataMember): No symbol table exists");
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

                    // for code generation, set the dataMember's entry to where the data resides
                    node->symbolTableEntry() = entry;
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

                // for code generation, set the dataMember's entry to where the data resides
                node->symbolTableEntry() = member;
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

void TypeCheckerVisitor::visit(ast::fCall* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("TypeCheckerVisitor::visit(fCall): No symbol table exists");
    }

    auto varNode = node->parent();
    auto idNode = dynamic_cast<ast::id*>(node->child(0));
    auto aParamsNode = dynamic_cast<ast::aParams*>(node->child(1));

    std::vector<VariableType> paramType;
    aParamsToVariableTypes(paramType, aParamsNode);

    if (!varNode->type()) {
        // this is the first member under var

        // check if this symbol has been previously declared in this scope (it should be a free function)
        if (auto entry = (*table)[idNode->token()->value]) {

            // find the var's type after this fCall is applied
            if (auto t = dynamic_cast<FunctionType*>(entry->type())) {
                // if we find it, set our type to the function's return type
                std::unique_ptr<VariableType> type(new VariableType(t->returnType));
                node->setType(std::unique_ptr<VariableType>(new VariableType(*type)));
                varNode->setType(std::move(type));

                // hack: make aParams hold a tempvar for the result of this function call
                node->child(1)->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
                node->child(1)->symbolTableEntry()->setName(nextTempVar());
                node->child(1)->symbolTableEntry()->setKind(SymbolTableEntryKind::TEMPVAR);
                node->child(1)->symbolTableEntry()->setType(std::unique_ptr<VariableType>(new VariableType(*node->type())));
                table->addEntry(node->child(1)->symbolTableEntry());

                // for code generation, set the dataMember's entry to where the data resides
                node->symbolTableEntry() = entry;
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

        if (!member || !member->parentTable()->parentEntry()) {
            // the class is valid, but the function doesn't exist in the class
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::UNDECLARED_MEMBER_FUNCTION, idNode->token());
        } else if (auto t = dynamic_cast<FunctionType*>(member->type())) {
            // check that the parameters used match the function declaration
            if (paramType == t->parameterTypes) {
                // the member exists, is a function and the params match--OK!
                // set our type to its return type
                std::unique_ptr<VariableType> type(new VariableType(t->returnType));
                varNode->setType(std::move(type));

                // hack: make aParams hold a tempvar for the result of this function call
                node->child(1)->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
                node->child(1)->symbolTableEntry()->setName(nextTempVar());
                node->child(1)->symbolTableEntry()->setKind(SymbolTableEntryKind::TEMPVAR);
                node->child(1)->symbolTableEntry()->setType(std::unique_ptr<VariableType>(new VariableType(*varNode->type())));
                table->addEntry(node->child(1)->symbolTableEntry());

                // for code generation, set the dataMember's entry to where the data resides
                node->symbolTableEntry() = member;
            } else {
                // there is a parameter mismatch
                std::unique_ptr<VariableType> type(new VariableType());
                type->type = Type::ERROR;
                varNode->setType(std::move(type));
                errors_->emplace_back(SemanticErrorType::INCORRECT_TYPE_IN_FUNCTION_CALL, idNode->token());
            }
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

void TypeCheckerVisitor::visit(ast::var* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(nextTempVar());
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::TEMPVAR);
    //node->symbolTableEntry()->setType(std::unique_ptr<VariableType>(new VariableType(*node->type())));
    std::unique_ptr<VariableType> type(new VariableType());
    type->type = Type::INT;
    node->symbolTableEntry()->setType(std::move(type));
    table->addEntry(node->symbolTableEntry());

    // TODO: merge this into TypeCheckerVisitor::visit(dataMember)?



    //node->symbolTableEntry() = (*table)[dynamic_cast<ast::id*>(node->child(0)->child(0))->token()->value];
}

void TypeCheckerVisitor::aParamsToVariableTypes(std::vector<VariableType>& types, const ast::aParams* node) const
{
    // loop over aParams and add their types to our types list
    for (auto aParam = node->child(); aParam != nullptr; aParam = aParam->next()) {
        if (!aParam->type()) {
            throw std::runtime_error("TypeCheckerVisitor::aParamsToVariableTypes: aParam does not have a type");
        }

        types.emplace_back(*aParam->type());
    }
}

std::string TypeCheckerVisitor::nextTempVar()
{
    return "_t" + std::to_string(currentTempVar_++);
}

}}
