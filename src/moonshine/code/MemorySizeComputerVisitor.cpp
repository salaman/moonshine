#include "moonshine/code/MemorySizeComputerVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace code {

using namespace semantic;

void MemorySizeComputerVisitor::visit(ast::prog* node)
{
    Visitor::visit(node);

    auto table = node->symbolTable();
    auto programEntry = (*table)["program"];
    auto programTable = programEntry->link();

    for (auto entry = programTable->begin(); entry != programTable->end(); ++entry) {
        programEntry->setSize(programEntry->size() + entry->second->size());
        entry->second->setOffset(programEntry->size());
    }

    programTable->setSize(programEntry->size());
}

void MemorySizeComputerVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);

    auto entry = node->symbolTableEntry();
    auto table = entry->parentTable();
    auto type = dynamic_cast<VariableType*>(entry->type());

    int size = 0;

    if (type->type == Type::CLASS) {
        std::shared_ptr<SymbolTableEntry> classEntry = (*table)[type->className];
        size = classEntry->size();
    } else {
        size = getPrimitiveSize(type->type);

        for (const auto& i : type->indices) {
            size *= i;
        }
    }

    entry->setSize(size);
}

void MemorySizeComputerVisitor::visit(ast::classDecl* node)
{
    Visitor::visit(node);

    auto table = node->symbolTable();

    for (auto entry = table->begin(); entry != table->end(); ++entry) {
        table->setSize(table->size() + entry->second->size());
        entry->second->setOffset(table->size());
    }

    node->symbolTableEntry()->setSize(table->size());
}

void MemorySizeComputerVisitor::visit(ast::funcDef* node)
{
    Visitor::visit(node);

    auto table = node->symbolTable();

    // stack frame contains the return value at the bottom of the stack
    table->setSize(table->size() + getPrimitiveSize(dynamic_cast<FunctionType*>(node->symbolTableEntry()->type())->returnType.type));

    // and the return address
    table->setSize(table->size() + 4);

    for (auto entry = table->begin(); entry != table->end(); ++entry) {
        table->setSize(table->size() + entry->second->size());
        entry->second->setOffset(table->size());
    }

    node->symbolTableEntry()->setSize(table->size());
}


void MemorySizeComputerVisitor::visit(ast::forStat* node)
{
    Visitor::visit(node);

    auto table = node->symbolTable();
    auto forVarDeclEntry = (*table)[dynamic_cast<ast::id*>(node->child(1))->token()->value];
    auto type = dynamic_cast<VariableType*>(forVarDeclEntry->type());

    int size = 0;

    if (type->type == Type::CLASS) {
        std::shared_ptr<SymbolTableEntry> classEntry = (*table)[type->className];
        size = classEntry->size();
    } else {
        size = getPrimitiveSize(type->type);

        for (const auto& i : type->indices) {
            size *= i;
        }
    }

    forVarDeclEntry->setSize(size);

    for (auto entry = table->begin(); entry != table->end(); ++entry) {
        table->setSize(table->size() + entry->second->size());
        entry->second->setOffset(table->size());
    }

    node->symbolTableEntry()->setSize(table->size());
}

void MemorySizeComputerVisitor::visit(ast::addOp* node)
{
    Visitor::visit(node);

    if (node->symbolTableEntry()) {
        node->symbolTableEntry()->setSize(getPrimitiveSize(node->type()->type));
    }
}

void MemorySizeComputerVisitor::visit(ast::multOp* node)
{
    Visitor::visit(node);

    if (node->symbolTableEntry()) {
        node->symbolTableEntry()->setSize(getPrimitiveSize(node->type()->type));
    }
}

void MemorySizeComputerVisitor::visit(ast::relOp* node)
{
    Visitor::visit(node);

    if (node->symbolTableEntry()) {
        node->symbolTableEntry()->setSize(getPrimitiveSize(dynamic_cast<VariableType*>(node->symbolTableEntry()->type())->type));
    }
}

void MemorySizeComputerVisitor::visit(ast::num* node)
{
    Visitor::visit(node);

    node->symbolTableEntry()->setSize(getPrimitiveSize(node->type()->type));
}

void MemorySizeComputerVisitor::visit(ast::var* node)
{
    Visitor::visit(node);

    //node->symbolTableEntry() = node->child()->symbolTableEntry();

    //node->symbolTableEntry() = std::make_shared<SymbolTableEntry>();
    //node->symbolTableEntry()->setName(node->child()->symbolTableEntry()->name());
    //node->symbolTableEntry()->setKind(node->child()->symbolTableEntry()->kind());
    ////node->symbolTableEntry()->setType(std::unique_ptr<SymbolType>(new SymbolType(*node->child()->symbolTableEntry()->type())));
    //node->symbolTableEntry()->setSize(getPrimitiveSize(node->type()->type));
    //node->symbolTableEntry()->setOffset(node->child()->relativeOffset);
    //table->addEntry(node->symbolTableEntry());

    //if (node->symbolTableEntry()) {
    //    node->symbolTableEntry()->setSize(getPrimitiveSize(node->type()->type));
    //}
}

int MemorySizeComputerVisitor::getPrimitiveSize(const semantic::Type& type)
{
    switch (type) {
        case Type::ERROR:
            return 0;
        case Type::INT:
            return 4;
        case Type::FLOAT:
            return 8;
        //case Type::CLASS:
        //    return 0;
        default:
            throw std::runtime_error("MemorySizeComputerVisitor::getPrimitiveSize: Invalid type given");
    }
}

void MemorySizeComputerVisitor::visit(ast::dataMember* node)
{
    Visitor::visit(node);
}

void MemorySizeComputerVisitor::visit(ast::fCall* node)
{
    Visitor::visit(node);

    if (node->symbolTableEntry()) {
        node->symbolTableEntry()->setSize(getPrimitiveSize(node->type()->type));
    }
}

}}
