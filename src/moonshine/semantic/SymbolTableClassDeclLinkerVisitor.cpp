#include "moonshine/semantic/SymbolTableClassDeclLinkerVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void SymbolTableClassDeclLinkerVisitor::visit(ast::classDecl* node)
{
    Visitor::visit(node);

    auto table = node->parent()->closestSymbolTable();

    if (!table) {
        throw std::runtime_error("SymbolTableClassDeclLinkerVisitor::visit(classDecl): No symbol table exists");
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

}}
