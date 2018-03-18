#include "moonshine/semantic/SymbolTableCreatorVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void SymbolTableCreatorVisitor::visit(ast::prog* node)
{
    Visitor::visit(node);
    std::cout << "SymbolTableCreatorVisitor::visit(prog)" << std::endl;

    node->symbolTable() = std::make_shared<SymbolTable>();

    // create entries for class declarations
    for (ast::Node* n = node->child(0)->child(); n != nullptr; n = n->next()) {
        if (n->symbolTableEntry()) {
            node->symbolTable()->addEntry(n->symbolTableEntry());
        }
    }

    // create entries for function definitions
    for (ast::Node* n = node->child(1)->child(); n != nullptr; n = n->next()) {
        if (n->symbolTableEntry()) {
            node->symbolTable()->addEntry(n->symbolTableEntry());
        }
    }

    // create entry for program function, using its symbol table
    auto program = std::make_shared<SymbolTableEntry>();
    program->setName("program");
    program->setKind(SymbolTableEntryKind::FUNCTION);
    program->setLink(node->child(2)->symbolTable());
    node->symbolTable()->addEntry(program);
}

void SymbolTableCreatorVisitor::visit(ast::assignStat* node)
{
    Visitor::visit(node);
    std::cout << "SymbolTableCreatorVisitor::visit(assignStat)" << std::endl;
}

void SymbolTableCreatorVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);
    std::cout << "SymbolTableCreatorVisitor::visit(varDecl)" << std::endl;

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
    std::cout << "SymbolTableCreatorVisitor::visit(statBlock)" << std::endl;

    node->symbolTable() = std::make_shared<SymbolTable>();

    for (ast::Node* n = node->child(); n != nullptr; n = n->next()) {
        if (n->symbolTableEntry()) {
            node->symbolTable()->addEntry(n->symbolTableEntry());
        }
    }
}

void SymbolTableCreatorVisitor::visit(ast::funcDecl* node)
{
    Visitor::visit(node);
    std::cout << "SymbolTableCreatorVisitor::visit(funcDecl)" << std::endl;

    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(dynamic_cast<ast::Leaf*>(node->child(1))->token()->value);
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::FUNCTION);

    std::unique_ptr<FunctionType> type(new FunctionType());
    nodeToFunctionType(*type, node);
    node->symbolTableEntry()->setType(std::move(type));
}

void SymbolTableCreatorVisitor::visit(ast::classDecl* node)
{
    Visitor::visit(node);
    std::cout << "SymbolTableCreatorVisitor::visit(classDecl)" << std::endl;

    // populate the symbol table entry for the class
    node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    node->symbolTableEntry()->setName(dynamic_cast<ast::Leaf*>(node->child(0))->token()->value);
    node->symbolTableEntry()->setKind(SymbolTableEntryKind::CLASS);

    // TODO: inherList

    // create the class' own symbol table
    node->symbolTableEntry()->setLink(std::make_shared<SymbolTable>());
    for (ast::Node* n = node->child(2)->child(); n != nullptr; n = n->next()) {
        if (n->symbolTableEntry()) {
            node->symbolTableEntry()->link()->addEntry(n->symbolTableEntry());
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

void SymbolTableCreatorVisitor::nodeToFunctionType(FunctionType& type, const ast::Node* node) const
{
    if (node == nullptr) {
        throw std::invalid_argument("SymbolTableCreatorVisitor::nodeToFunctionType: node cannot be nullptr");
    }

    auto typeNode = dynamic_cast<const ast::Leaf*>(node->child(0));

    if (typeNode == nullptr) {
        throw std::runtime_error("SymbolTableCreatorVisitor::nodeToFunctionType: First child is not a leaf node");
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
            throw std::runtime_error("SymbolTableCreatorVisitor::nodeToFunctionType: Invalid AST type node");
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

}}
