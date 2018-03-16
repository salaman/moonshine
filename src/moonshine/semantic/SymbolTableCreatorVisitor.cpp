#include "moonshine/semantic/SymbolTableCreatorVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <iostream>
#include <memory>

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

    auto typeNode = dynamic_cast<ast::Leaf*>(node->child(0));
    std::unique_ptr<VariableType> type(new VariableType());

    switch (typeNode->token()->type) {
        case TokenType::T_INT:
            type->type = Type::INT;
            break;
        case TokenType::T_FLOAT:
            type->type = Type::FLOAT;
            break;
        case TokenType::T_IDENTIFIER:
            type->type = Type::CLASS;
            type->className = typeNode->token()->value;
            break;
        default:
            throw std::runtime_error("Invalid AST type node");
    }

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

    auto typeNode = dynamic_cast<ast::Leaf*>(node->child(0));
    std::unique_ptr<FunctionType> type(new FunctionType());

    switch (typeNode->token()->type) {
        case TokenType::T_INT:
            type->returnType.type = Type::INT;
            break;
        case TokenType::T_FLOAT:
            type->returnType.type = Type::FLOAT;
            break;
        case TokenType::T_IDENTIFIER:
            type->returnType.type = Type::CLASS;
            type->returnType.className = typeNode->token()->value;
            break;
        default:
            throw std::runtime_error("Invalid AST type node");
    }

    for (ast::Node* n = node->child(2)->child(); n != nullptr; n = n->next()) {
        typeNode = dynamic_cast<ast::Leaf*>(n->child(0));
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

        type->parameterTypes.emplace_back(std::move(parameterType));
    }

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

}}
