#include "moonshine/semantic/SymbolTableCreatorVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void SymbolTableCreatorVisitor::visit(ast::prog* node)
{
    Visitor::visit(node);

    // create global symbol table
    auto table = node->symbolTable() = std::make_shared<SymbolTable>();

    // create entries for class declarations
    for (ast::Node* n = node->child(0)->child(); n != nullptr; n = n->next()) {
        if (n->symbolTableEntry()) {
            // TODO: redeclaration check

            node->symbolTable()->addEntry(n->symbolTableEntry());
        }
    }

    // create entries for function definitions
    for (ast::Node* n = node->child(1)->child(); n != nullptr; n = n->next()) {
        if (n->symbolTableEntry()) {
            // TODO: redeclaration check
            // TODO: check parameters match

            auto type = dynamic_cast<FunctionType*>(n->symbolTableEntry()->type());

            if (type->scope.empty()) {
                // this function definition is a free function in the global symbol table
                node->symbolTable()->addEntry(n->symbolTableEntry());
            } else {
                // this function definition is for a class member function
                auto classTable = (*table)[type->scope]->link();

                // find the function entry in the class' symbol table
                if (auto entry = (*classTable)[n->symbolTableEntry()->name()]) {
                    entry->setLink(n->symbolTableEntry()->link());
                } else {
                    errors_->emplace_back(SemanticErrorType::UNDECLARED_FUNCTION);
                }
            }
        }
    }

    // TODO: check that all functions have been defined

    // create entry for program function, using its symbol table
    auto program = std::make_shared<SymbolTableEntry>();
    program->setName("program");
    program->setKind(SymbolTableEntryKind::FUNCTION);
    //program->setLink(node->child(2)->symbolTable());
    std::shared_ptr<SymbolTable> programTable = std::make_shared<SymbolTable>();
    statBlockToSymbolTable(*programTable, dynamic_cast<ast::statBlock*>(node->child(2)));
    program->setLink(programTable);
    node->symbolTable()->addEntry(program);
}

void SymbolTableCreatorVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);

    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(dynamic_cast<ast::Leaf*>(node->child(1))->token()->value);
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::VARIABLE);

    std::unique_ptr<VariableType> type(new VariableType());
    nodeToVariableType(*type, node);
    node->symbolTableEntry()->setType(std::move(type));
}

void SymbolTableCreatorVisitor::visit(ast::statBlock* node)
{
    Visitor::visit(node);

    // TODO: var decl checks go here



    //auto table = node->closestSymbolTable();
    //
    //if (!table) {
    //    throw std::runtime_error("SymbolTableCreatorVisitor::statBlockToSymbolTable: No symbol table exists");
    //}
    //
    //statBlockToSymbolTable(*table, node);

    //// create a new symbol table for this scope
    //std::shared_ptr<SymbolTable> table = node->symbolTable() = std::make_shared<SymbolTable>();
    //
    //// merge entries for each child node (ie. statement)
    //for (ast::Node* n = node->child(); n != nullptr; n = n->next()) {
    //    if (n->symbolTableEntry()) {
    //        // check if this symbol has been previously declared in this scope
    //        if ((*table)[n->symbolTableEntry()->name()]) {
    //            errors_->emplace_back(SemanticErrorType::REDECLARED_SYMBOL);
    //        } else {
    //            table->addEntry(n->symbolTableEntry());
    //        }
    //    }
    //}
}

void SymbolTableCreatorVisitor::visit(ast::funcDecl* node)
{
    Visitor::visit(node);

    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(dynamic_cast<ast::Leaf*>(node->child(1))->token()->value);
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::FUNCTION);

    std::unique_ptr<FunctionType> type(new FunctionType());
    funcDeclToFunctionType(*type, node);
    node->symbolTableEntry()->setType(std::move(type));
}

void SymbolTableCreatorVisitor::visit(ast::classDecl* node)
{
    Visitor::visit(node);

    // populate the symbol table entry for the class
    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(dynamic_cast<ast::Leaf*>(node->child(0))->token()->value);
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::CLASS);

    // TODO: inherList

    // create the class' own symbol table
    node->symbolTableEntry()->setLink(std::make_shared<SymbolTable>());

    // membList
    for (ast::Node* n = node->child(2)->child(); n != nullptr; n = n->next()) {
        if (n->symbolTableEntry()) {
            // add this member (varDecl or funcDecl) entry to the class symbol table
            node->symbolTableEntry()->link()->addEntry(n->symbolTableEntry());
        }
    }
}

void SymbolTableCreatorVisitor::visit(ast::funcDef* node)
{
    Visitor::visit(node);

    // populate the symbol table entry for the function
    auto entry = node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();

    // if the 3rd child is not nul, we know the 2nd child is a scope
    auto nameNode = dynamic_cast<ast::Leaf*>(node->child(2))
                    ? dynamic_cast<ast::Leaf*>(node->child(2))
                    : dynamic_cast<ast::Leaf*>(node->child(1));
    entry->setName(nameNode->token()->value);
    entry->setKind(SymbolTableEntryKind::FUNCTION);

    // set the function's type in the entry
    std::unique_ptr<FunctionType> type(new FunctionType());
    funcDefToFunctionType(*type, node);
    entry->setType(std::move(type));

    // create the function symbol table
    std::shared_ptr<SymbolTable> table = std::make_shared<SymbolTable>();

    // add the function parameters to the symbol table
    fparamListToSymbolTable(*table, dynamic_cast<ast::fparamList*>(node->child(3)));

    // add the function's statBlock symbols to the table
    statBlockToSymbolTable(*table, dynamic_cast<ast::statBlock*>(node->child(4)));
    entry->setLink(table);
}

void SymbolTableCreatorVisitor::statBlockToSymbolTable(SymbolTable& table, ast::statBlock* node) const
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

void SymbolTableCreatorVisitor::nodeToVariableType(VariableType& type, const ast::Node* node) const
{
    if (node == nullptr) {
        throw std::invalid_argument("SymbolTableCreatorVisitor::nodeToVar: node cannot be nullptr");
    }

    auto typeNode = dynamic_cast<const ast::Leaf*>(node->child(0));

    if (typeNode == nullptr) {
        throw std::runtime_error("SymbolTableCreatorVisitor::nodeToVar: First child is not a leaf node");
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
            throw std::runtime_error("SymbolTableCreatorVisitor::nodeToVar: Invalid AST type node");
    }

    // dimList
    for (auto n = dynamic_cast<const ast::num*>(node->child(2)->child()); n != nullptr; n = dynamic_cast<const ast::num*>(n->next())) {
        type.indices.emplace_back(std::stoi(n->token()->value));
    }
}

void SymbolTableCreatorVisitor::funcDeclToFunctionType(FunctionType& type, const ast::Node* node) const
{
    if (node == nullptr) {
        throw std::invalid_argument("SymbolTableCreatorVisitor::funcDeclToFunctionType: node cannot be nullptr");
    }

    auto typeNode = dynamic_cast<const ast::Leaf*>(node->child(0));

    if (typeNode == nullptr) {
        throw std::runtime_error("SymbolTableCreatorVisitor::funcDeclToFunctionType: First child is not a leaf node");
    }

    switch (typeNode->token()->type) {
        case TokenType::T_INT:
            type.returnType.type = Type::INT;
            break;
        case TokenType::T_FLOAT:
            type.returnType.type = Type::FLOAT;
            break;
        case TokenType::T_IDENTIFIER:
            type.returnType.type = Type::CLASS;
            type.returnType.className = typeNode->token()->value;
            break;
        default:
            throw std::runtime_error("SymbolTableCreatorVisitor::funcDeclToFunctionType: Invalid AST type node");
    }

    // fparam
    for (auto fparam = dynamic_cast<const ast::fparam*>(node->child(2)->child()); fparam != nullptr; fparam = dynamic_cast<const ast::fparam*>(fparam->next())) {
        typeNode = dynamic_cast<ast::Leaf*>(fparam->child(0));
        VariableType parameterType;

        switch (typeNode->token()->type) {
            case TokenType::T_INT:
                parameterType.type = Type::INT;
                break;
            case TokenType::T_FLOAT:
                parameterType.type = Type::FLOAT;
                break;
            case TokenType::T_IDENTIFIER:
                parameterType.type = Type::CLASS;
                parameterType.className = typeNode->token()->value;
                break;
            default:
                throw std::runtime_error("Invalid AST type node");
        }

        // dimList
        for (auto num = dynamic_cast<const ast::num*>(fparam->child(2)->child()); num != nullptr; num = dynamic_cast<const ast::num*>(num->next())) {
            parameterType.indices.emplace_back(std::stoi(num->token()->value));
        }

        type.parameterTypes.emplace_back(parameterType);
    }
}

void SymbolTableCreatorVisitor::funcDefToFunctionType(FunctionType& type, const ast::Node* node) const
{
    if (node == nullptr) {
        throw std::invalid_argument("SymbolTableCreatorVisitor::funcDefToFunctionType: node cannot be nullptr");
    }

    auto typeNode = dynamic_cast<const ast::Leaf*>(node->child(0));

    if (typeNode == nullptr) {
        throw std::runtime_error("SymbolTableCreatorVisitor::funcDefToFunctionType: First child is not a leaf node");
    }

    switch (typeNode->token()->type) {
        case TokenType::T_INT:
            type.returnType.type = Type::INT;
            break;
        case TokenType::T_FLOAT:
            type.returnType.type = Type::FLOAT;
            break;
        case TokenType::T_IDENTIFIER:
            type.returnType.type = Type::CLASS;
            type.returnType.className = typeNode->token()->value;
            break;
        default:
            throw std::runtime_error("SymbolTableCreatorVisitor::funcDefToFunctionType: Invalid AST type node");
    }

    // scope
    // if the 3rd child is not nul, we know the 2nd child is a scope
    if (!dynamic_cast<ast::nul*>(node->child(2))) {
        typeNode = dynamic_cast<ast::Leaf*>(node->child(1));
        type.scope = typeNode->token()->value;
    }

    // fparam
    for (auto fparam = dynamic_cast<const ast::fparam*>(node->child(3)->child()); fparam != nullptr; fparam = dynamic_cast<const ast::fparam*>(fparam->next())) {
        typeNode = dynamic_cast<ast::Leaf*>(fparam->child(0));
        VariableType parameterType;

        switch (typeNode->token()->type) {
            case TokenType::T_INT:
                parameterType.type = Type::INT;
                break;
            case TokenType::T_FLOAT:
                parameterType.type = Type::FLOAT;
                break;
            case TokenType::T_IDENTIFIER:
                parameterType.type = Type::CLASS;
                parameterType.className = typeNode->token()->value;
                break;
            default:
                throw std::runtime_error("Invalid AST type node");
        }

        // dimList
        for (auto num = dynamic_cast<const ast::num*>(fparam->child(2)->child()); num != nullptr; num = dynamic_cast<const ast::num*>(num->next())) {
            parameterType.indices.emplace_back(std::stoi(num->token()->value));
        }

        type.parameterTypes.emplace_back(parameterType);
    }
}

void SymbolTableCreatorVisitor::fparamListToSymbolTable(SymbolTable& table, ast::fparamList* node) const
{
    // merge entries for each child fparam
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

void SymbolTableCreatorVisitor::visit(ast::fparam* node)
{
    Visitor::visit(node);

    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(dynamic_cast<ast::Leaf*>(node->child(1))->token()->value);
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::PARAMETER);

    std::unique_ptr<VariableType> type(new VariableType());
    nodeToVariableType(*type, node);
    node->symbolTableEntry()->setType(std::move(type));
}

}}
