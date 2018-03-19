#include "moonshine/semantic/StatementBlockDeclVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void StatementBlockDeclVisitor::visit(ast::prog* node)
{
    Visitor::visit(node);
}

void StatementBlockDeclVisitor::visit(ast::statBlock* node)
{
    Visitor::visit(node);
}

void StatementBlockDeclVisitor::statBlockToSymbolTable(SymbolTable& table, ast::statBlock* node) const
{
    // merge entries for each child node (ie. statement)
    for (ast::Node* n = node->child(); n != nullptr; n = n->next()) {
        if (n->symbolTableEntry()) {
            // check if this symbol has been previously declared in this scope
            if (table[n->symbolTableEntry()->name()]) {
                errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL);
            } else {
                table.addEntry(n->symbolTableEntry());
            }
        }
    }
}

void StatementBlockDeclVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("StatementBlockDeclVisitor::visit(statBlock): No symbol table exists");
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

void StatementBlockDeclVisitor::visit(ast::forStat* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("StatementBlockDeclVisitor::visit(statBlock): No symbol table exists");
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

void StatementBlockDeclVisitor::nodeToVariableType(VariableType& type, const ast::Node* node) const
{
    if (node == nullptr) {
        throw std::invalid_argument("StatementBlockDeclVisitor::nodeToVar: node cannot be nullptr");
    }

    auto typeNode = dynamic_cast<const ast::Leaf*>(node->child(0));

    if (typeNode == nullptr) {
        throw std::runtime_error("StatementBlockDeclVisitor::nodeToVar: First child is not a leaf node");
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
            throw std::runtime_error("StatementBlockDeclVisitor::nodeToVar: Invalid AST type node");
    }

    // dimList
    for (auto n = dynamic_cast<const ast::num*>(node->child(2)->child()); n != nullptr; n = dynamic_cast<const ast::num*>(n->next())) {
        type.indices.emplace_back(std::stoi(n->token()->value));
    }
}

void StatementBlockDeclVisitor::visit(ast::funcDecl* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("StatementBlockDeclVisitor::visit(funcDecl): No symbol table exists");
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

void StatementBlockDeclVisitor::visit(ast::fparamList* node)
{
    Visitor::visit(node);
}

void StatementBlockDeclVisitor::visit(ast::classDecl* node)
{
    Visitor::visit(node);

    auto table = node->parent()->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("StatementBlockDeclVisitor::visit(classDecl): No symbol table exists");
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

void StatementBlockDeclVisitor::visit(ast::funcDef* node)
{
    Visitor::visit(node);

    auto parentTable = node->parent()->closestSymbolTable();

    if (!parentTable) {
        throw std::runtime_error("StatementBlockDeclVisitor::visit(funcDef): No symbol table exists");
    }

    auto type = dynamic_cast<FunctionType*>(node->symbolTableEntry()->type());

    // TODO: check parameters match, overloading?

    if (type->scope.empty()) {
        // this function definition is a free function in the global symbol table
        // TODO: redecl check
        parentTable->addEntry(node->symbolTableEntry());
    } else {
        // this function definition is for a class member function
        auto classTable = (*parentTable)[type->scope]->link();

        // find the function entry in the class' symbol table
        if (auto entry = (*classTable)[node->symbolTableEntry()->name()]) {
            entry->setLink(node->symbolTableEntry()->link());
        } else {
            errors_->emplace_back(SemanticErrorType::UNDECLARED_FUNCTION);
        }
    }
}

void StatementBlockDeclVisitor::visit(ast::var* node)
{
    Visitor::visit(node);


}

}}
