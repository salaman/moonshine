#include "moonshine/code/MemorySizeComputerVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>
#include <map>
//#include <iostream>

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
        // ignore varDecl within classes, those are dealt with in the classList visitor
        if (!dynamic_cast<ast::membList*>(node->parent())) {
            std::shared_ptr<SymbolTableEntry> classEntry = (*table)[type->className];
            size = classEntry->size();
        }
    } else {
        size = getPrimitiveSize(type->type);

        for (const auto& i : type->indices) {
            size *= i;
        }
    }

    entry->setSize(size);
}

void MemorySizeComputerVisitor::visit(ast::fparam* node)
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

void MemorySizeComputerVisitor::visit(ast::classList* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    // calculate sizes of class tables w/ class members

    std::map<std::string, int> sizes;

    while (true) {
        bool done = true;

        for (auto it = table->begin(); it != table->end(); ++it) {
            std::shared_ptr<SymbolTableEntry> classEntry = it->second;

            if (classEntry->kind() != SymbolTableEntryKind::CLASS) {
                continue;
            }

            // if we've already successfully processed this class, skip it
            if (sizes.find(classEntry->name()) != sizes.end()) {
                continue;
            }

            int size = 0;
            bool leaf = true;

            // for all variable entries of the class' symbol table...
            for (auto& entry : *classEntry->link()) {
                if (entry.second->kind() != SymbolTableEntryKind::VARIABLE) {
                    continue;
                }

                auto type = dynamic_cast<VariableType*>(entry.second->type());

                if (type->type != Type::CLASS) {
                    // for non-class members, we already have the size
                    size += entry.second->size();
                    entry.second->setOffset(size);
                } else if (sizes.find(type->className) != sizes.end()) {
                    // for class members, add the size if we already know it
                    size += sizes[type->className];
                    entry.second->setSize(sizes[type->className]);
                    entry.second->setOffset(size);
                } else {
                    // this is an unprocessed entry
                    leaf = false;
                    done = false;
                    break;
                }
            }

            if (leaf) {
                //std::cout << classEntry->name() << " = " << size << std::endl;
                classEntry->setSize(size);
                classEntry->link()->setSize(size);
                sizes[classEntry->name()] = size;
            }
        }

        if (done) {
            break;
        }
    }

    // calculate sizes of class entries w/ inheritance

    for (auto it = table->begin(); it != table->end(); ++it) {
        std::shared_ptr<SymbolTableEntry> classEntry = it->second;

        if (classEntry->kind() != SymbolTableEntryKind::CLASS) {
            continue;
        }

        for (const auto& super : classEntry->supers()) {
            classEntry->setSize(classEntry->size() + super->size());
        }
    }
}

void MemorySizeComputerVisitor::visit(ast::funcDef* node)
{
    Visitor::visit(node);

    auto table = node->symbolTable();
    auto type = dynamic_cast<FunctionType*>(node->symbolTableEntry()->type());

    // stack frame contains the return value at the bottom of the stack
    int returnSize = 0;

    if (type->returnType.type == Type::CLASS) {
        auto classEntry = (*table)[type->returnType.className];
        returnSize = classEntry->size();
    } else {
        returnSize = getPrimitiveSize(type->returnType.type);
    }

    table->setSize(table->size() + returnSize);

    // and the return address
    table->setSize(table->size() + getPrimitiveSize(Type::INT));

    for (auto entry = table->begin(); entry != table->end(); ++entry) {
        table->setSize(table->size() + entry->second->size());
        entry->second->setOffset(table->size());
    }
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

    if (node->symbolTableEntry()) {
        node->symbolTableEntry()->setSize(getPrimitiveSize(node->type()->type));
    }
}

void MemorySizeComputerVisitor::visit(ast::var* node)
{
    Visitor::visit(node);

    if (node->symbolTableEntry()) {
        node->symbolTableEntry()->setSize(getPrimitiveSize(Type::INT));
    }
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

void MemorySizeComputerVisitor::visit(ast::aParams* node)
{
    Visitor::visit(node);

    if (!node->symbolTableEntry()) {
        return;
    }

    auto table = node->closestSymbolTable();
    auto type = dynamic_cast<VariableType*>(node->symbolTableEntry()->type());

    int returnSize = 0;

    if (type->type == Type::CLASS) {
        auto classEntry = (*table)[type->className];
        returnSize = classEntry->size();
    } else {
        returnSize = getPrimitiveSize(type->type);
    }

    node->symbolTableEntry()->setSize(returnSize);
}

}}
