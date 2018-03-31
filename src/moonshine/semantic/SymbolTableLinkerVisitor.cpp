#include "moonshine/semantic/SymbolTableLinkerVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void SymbolTableLinkerVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("SymbolTableLinkerVisitor::visit(statBlock): No symbol table exists");
    }

    if (node->symbolTableEntry()) {
        // check if this symbol has been previously declared in this table
        // don't go up the table hierarchy as to not handle shadowing, which is done later
        if (table->get(node->symbolTableEntry()->name())) {
            node->marked = true;
            errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL, dynamic_cast<ast::id*>(node->child(1))->token());
        } else {
            table->addEntry(node->symbolTableEntry());
        }
    }
}

void SymbolTableLinkerVisitor::visit(ast::forStat* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("SymbolTableLinkerVisitor::visit(statBlock): No symbol table exists");
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

    // add the variable to the forStat's symbol table
    table->addEntry(varDecl);
}

void SymbolTableLinkerVisitor::nodeToVariableType(VariableType& type, const ast::Node* node) const
{
    if (node == nullptr) {
        throw std::invalid_argument("SymbolTableLinkerVisitor::nodeToVar: node cannot be nullptr");
    }

    auto typeNode = dynamic_cast<const ast::Leaf*>(node->child(0));

    if (typeNode == nullptr) {
        throw std::runtime_error("SymbolTableLinkerVisitor::nodeToVar: First child is not a leaf node");
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
            throw std::runtime_error("SymbolTableLinkerVisitor::nodeToVar: Invalid AST type node");
    }

    // dimList
    for (auto n = dynamic_cast<const ast::num*>(node->child(2)->child()); n != nullptr; n = dynamic_cast<const ast::num*>(n->next())) {
        type.indices.emplace_back(std::stoi(n->token()->value));
    }
}

void SymbolTableLinkerVisitor::visit(ast::funcDecl* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("SymbolTableLinkerVisitor::visit(funcDecl): No symbol table exists");
    }

    if (node->symbolTableEntry()) {
        // check if this symbol has been previously declared in this scope
        if ((*table)[node->symbolTableEntry()->name()]) {
            errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL, dynamic_cast<ast::id*>(node->child(1))->token());
        } else {
            table->addEntry(node->symbolTableEntry());
        }
    }
}

void SymbolTableLinkerVisitor::visit(ast::classDecl* node)
{
    Visitor::visit(node);

    auto table = node->parent()->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("SymbolTableLinkerVisitor::visit(classDecl): No symbol table exists");
    }

    if (node->symbolTableEntry()) {
        // check if this symbol has been previously declared in this scope
        if ((*table)[node->symbolTableEntry()->name()]) {
            errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL, dynamic_cast<ast::id*>(node->child(0))->token());
        } else {
            table->addEntry(node->symbolTableEntry());
        }
    }
}

void SymbolTableLinkerVisitor::visit(ast::funcDef* node)
{
    Visitor::visit(node);

    auto parentTable = node->parent()->closestSymbolTable();

    if (!parentTable) {
        throw std::runtime_error("SymbolTableLinkerVisitor::visit(funcDef): No symbol table exists");
    }

    auto type = dynamic_cast<FunctionType*>(node->symbolTableEntry()->type());

    if (type->scope.empty()) {
        // this function definition is a free function in the global symbol table

        if ((*parentTable)[node->symbolTableEntry()->name()]) {
            // an entry for this function already exists
            errors_->emplace_back(SemanticErrorType::REDEFINED_FUNCTION, dynamic_cast<ast::id*>(node->child(1))->token());
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
                errors_->emplace_back(SemanticErrorType::REDEFINED_FUNCTION, dynamic_cast<ast::id*>(node->child(2))->token());
            } else if (*type != *entry->type()) {
                // the function exists, but the definition differs from the declaration
                errors_->emplace_back(SemanticErrorType::INCORRECT_TYPE_IN_FUNCTION_DEFINITION, dynamic_cast<ast::id*>(node->child(2))->token());
            } else {
                // else, set the class's function entry's symbol table to this one
                entry->setLink(node->symbolTable());

                // and set funcDef's entry to the new one (the old one is abandoned)
                // this is so future visitors can properly refer to this node's entry
                node->symbolTableEntry() = entry;
            }

        } else {
            errors_->emplace_back(SemanticErrorType::UNDECLARED_FUNCTION, dynamic_cast<ast::id*>(node->child(2))->token());
        }
    }
}

void SymbolTableLinkerVisitor::visit(ast::returnStat* node)
{
    Visitor::visit(node);

    // find the symbol table for the function we're in
    auto table = node->closestSymbolTable().get();
    while (table && table->parentEntry()->kind() != SymbolTableEntryKind::FUNCTION) {
        table = table->parentEntry()->parentTable();
    }

    if (!table) {
        throw std::runtime_error("SymbolTableLinkerVisitor::visit(returnStat): No symbol table exists");
    }

    // mark return exists
    table->parentEntry()->setHasReturn(true);
}

}}
