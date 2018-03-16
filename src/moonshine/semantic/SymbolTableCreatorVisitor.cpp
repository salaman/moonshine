#include "moonshine/semantic/SymbolTableCreatorVisitor.h"

#include <iostream>

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
    node->symbolTableEntry()->setType(dynamic_cast<ast::Leaf*>(node->child(0))->token()->value);
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
    node->symbolTableEntry()->setType(dynamic_cast<ast::Leaf*>(node->child(0))->token()->value);
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
