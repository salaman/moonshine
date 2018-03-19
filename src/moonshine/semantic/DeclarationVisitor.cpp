#include "moonshine/semantic/DeclarationVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void DeclarationVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("DeclarationVisitor::visit(statBlock): No symbol table exists");
    }

    if (node->symbolTableEntry()) {
        // check if this symbol has been previously declared in this scope
        if ((*table)[node->symbolTableEntry()->name()]) {
            errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL);
        } else {
            table->addEntry(node->symbolTableEntry());
        }
    }
}

void DeclarationVisitor::visit(ast::forStat* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("DeclarationVisitor::visit(statBlock): No symbol table exists");
    }

    // link up
    node->parent()->closestSymbolTable()->addEntry(node->symbolTableEntry());

    // create a new entry for the built-in for variable declaration
    auto varDecl = std::make_shared<SymbolTableEntry>();
    varDecl->setName(dynamic_cast<ast::Leaf*>(node->child(1))->token()->value);
    varDecl->setKind(SymbolTableEntryKind::VARIABLE);

    std::unique_ptr<VariableType> type(new VariableType());
    nodeToVariableType(*type, node);
    varDecl->setType(std::move(type));

    // check if this symbol has been previously declared in this scope
    if ((*table)[varDecl->name()]) {
        errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL);
    } else {
        // add the variable to the forStat's symbol table
        table->addEntry(varDecl);
    }
}

void DeclarationVisitor::nodeToVariableType(VariableType& type, const ast::Node* node) const
{
    if (node == nullptr) {
        throw std::invalid_argument("DeclarationVisitor::nodeToVar: node cannot be nullptr");
    }

    auto typeNode = dynamic_cast<const ast::Leaf*>(node->child(0));

    if (typeNode == nullptr) {
        throw std::runtime_error("DeclarationVisitor::nodeToVar: First child is not a leaf node");
    }

    // type (node is either type or id)
    switch (typeNode->token()->type) {
        case TokenType::T_INT:
            type.type = Type::INT;
            break;
        case TokenType::T_FLOAT:
            type.type = Type::FLOAT;
            break;
        case TokenType::T_IDENTIFIER:
            type.type = Type::CLASS;
            type.className = typeNode->token()->value;
            break;
        default:
            throw std::runtime_error("DeclarationVisitor::nodeToVar: Invalid AST type node");
    }

    // dimList
    for (auto n = dynamic_cast<const ast::num*>(node->child(2)->child()); n != nullptr; n = dynamic_cast<const ast::num*>(n->next())) {
        type.indices.emplace_back(std::stoi(n->token()->value));
    }
}

void DeclarationVisitor::visit(ast::funcDecl* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("DeclarationVisitor::visit(funcDecl): No symbol table exists");
    }

    if (node->symbolTableEntry()) {
        // check if this symbol has been previously declared in this scope
        if ((*table)[node->symbolTableEntry()->name()]) {
            errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL);
        } else {
            table->addEntry(node->symbolTableEntry());
        }
    }
}

void DeclarationVisitor::visit(ast::classDecl* node)
{
    Visitor::visit(node);

    auto table = node->parent()->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("DeclarationVisitor::visit(classDecl): No symbol table exists");
    }

    if (node->symbolTableEntry()) {
        // check if this symbol has been previously declared in this scope
        if ((*table)[node->symbolTableEntry()->name()]) {
            errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL);
        } else {
            table->addEntry(node->symbolTableEntry());
        }
    }
}

void DeclarationVisitor::visit(ast::funcDef* node)
{
    Visitor::visit(node);

    auto parentTable = node->parent()->closestSymbolTable();

    if (!parentTable) {
        throw std::runtime_error("DeclarationVisitor::visit(funcDef): No symbol table exists");
    }

    auto type = dynamic_cast<FunctionType*>(node->symbolTableEntry()->type());

    // TODO: check parameters match, overloading?

    if (type->scope.empty()) {
        // this function definition is a free function in the global symbol table

        if ((*parentTable)[node->symbolTableEntry()->name()]) {
            // an entry for this function already exists
            // TODO: signature
            errors_->emplace_back(SemanticErrorType::REDEFINED_FUNCTION);
        } else {
            // this is the first definition
            parentTable->addEntry(node->symbolTableEntry());
        }
    } else {
        // this function definition is for a class member function
        auto classTable = (*parentTable)[type->scope]->link();

        // find the function entry in the class' symbol table
        if (auto entry = (*classTable)[node->symbolTableEntry()->name()]) {

            // check if a symbol table exists for this entry already
            if (entry->link()) {
                // if it does, this is a redefinition
                // TODO: signature
                errors_->emplace_back(SemanticErrorType::REDEFINED_FUNCTION);
            } else {
                // else, set the class's function entry's symbol table to this one
                entry->setLink(node->symbolTable());

                // and set funcDef's entry to the new one (the old one is abandoned and unused)
                // this is so future visitors can properly refer to this node's entry
                node->symbolTableEntry() = entry;
            }

        } else {
            errors_->emplace_back(SemanticErrorType::UNDECLARED_FUNCTION);
        }
    }
}

void DeclarationVisitor::visit(ast::dataMember* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("DeclarationVisitor::visit(dataMember): No symbol table exists");
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
                    errors_->emplace_back(SemanticErrorType::INVALID_DIMENSION_COUNT);
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
                errors_->emplace_back(SemanticErrorType::INVALID_VARIABLE);
            }

        } else {
            // the id was not found in the symbol table
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::UNDECLARED_VARIABLE);
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
            errors_->emplace_back(SemanticErrorType::UNDECLARED_MEMBER_VARIABLE);
        } else if (auto t = dynamic_cast<VariableType*>(member->type())) {
            // the member exists and is a variable
            // set our type to a copy of its type
            std::unique_ptr<VariableType> type(new VariableType());
            auto indiceCount = indexListNode->childCount();

            // check that the number of indices matches the dimension of the original type
            if (t->indices.size() != indiceCount) {
                // if not, this is an error
                type->type = Type::ERROR;
                errors_->emplace_back(SemanticErrorType::INVALID_DIMENSION_COUNT);
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
            errors_->emplace_back(SemanticErrorType::INVALID_VARIABLE);
        }

    } else if (varNode->type()->type == Type::ERROR) {
        // don't process errors
    } else {
        // an int or float terminates the chain, therefore this data member is invalid
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        varNode->setType(std::move(type));
        errors_->emplace_back(SemanticErrorType::INVALID_VARIABLE);
    }
}

void DeclarationVisitor::visit(ast::fCall* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("DeclarationVisitor::visit(fCall): No symbol table exists");
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
                node->marked = true;
                node->setType(std::unique_ptr<VariableType>(new VariableType(*type)));
                varNode->setType(std::move(type));
            } else {
                // the symbol exists, but is not a function
                std::unique_ptr<VariableType> type(new VariableType());
                type->type = Type::ERROR;
                varNode->setType(std::move(type));
                errors_->emplace_back(SemanticErrorType::INVALID_FUNCTION);
            }

        } else {
            // the id was not found in the symbol table
            std::unique_ptr<VariableType> type(new VariableType());
            type->type = Type::ERROR;
            varNode->setType(std::move(type));
            errors_->emplace_back(SemanticErrorType::UNDECLARED_FUNCTION);
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
            errors_->emplace_back(SemanticErrorType::UNDECLARED_MEMBER_FUNCTION);
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
            errors_->emplace_back(SemanticErrorType::INVALID_FUNCTION);
        }

    } else if (varNode->type()->type == Type::ERROR) {
        // don't process errors
    } else {
        // an int or float terminates the chain, therefore this data member is invalid
        std::unique_ptr<VariableType> type(new VariableType());
        type->type = Type::ERROR;
        varNode->setType(std::move(type));
        errors_->emplace_back(SemanticErrorType::INVALID_FUNCTION);
    }
}

void DeclarationVisitor::visit(ast::returnStat* node)
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

    if (!type) {
        throw std::runtime_error("TypeCheckVisitor::visit(returnStat): Invalid function type");
    }

    // mark return exists
    table->parentEntry()->setHasReturn(true);
}

}}
