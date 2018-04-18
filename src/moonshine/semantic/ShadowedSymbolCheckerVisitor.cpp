#include "moonshine/semantic/ShadowedSymbolCheckerVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void ShadowedSymbolCheckerVisitor::visit(ast::varDecl* node)
{
    Visitor::visit(node);

    auto entry = node->symbolTableEntry();

    // ignore any varDecl nodes that aren't being used (multiply declared, etc.)
    if (!entry || !entry->parentTable()) {
        return;
    }

    // get the local table and the parent table
    auto table = entry->parentTable();
    auto parentTable = table->parentEntry()->parentTable();

    // if there's no parent table, do nothing
    // this happens if eg. a function definition was incorrect and never linked to its declaraction
    if (!parentTable) {
        return;
    }

    // shadow-check this varDecl by seeing if it exists in this table and not the parent table
    if ((*parentTable)[entry->name()]) {
        errors_->emplace_back(SemanticErrorType::SHADOWED_VARIABLE, dynamic_cast<ast::id*>(node->child(1))->token(), SemanticErrorLevel::WARN);
        node->marked = true;
    }
}

void ShadowedSymbolCheckerVisitor::visit(ast::forStat* node)
{
    Visitor::visit(node);

    auto entry = node->symbolTableEntry();

    // ignore any forStat nodes that aren't being used
    if (!entry || !entry->parentTable()) {
        return;
    }

    // get the local table and the parent table
    auto table = entry->parentTable();
    auto parentTable = table->parentEntry()->parentTable();

    if (!parentTable) {
        return;
    }

    // shadow-check this varDecl by seeing if it exists in this table and not the parent table
    auto id = dynamic_cast<ast::id*>(node->child(1));
    if ((*parentTable)[id->token()->value]) {
        errors_->emplace_back(SemanticErrorType::SHADOWED_VARIABLE, id->token(), SemanticErrorLevel::WARN);
        node->marked = true;
    }
}

void ShadowedSymbolCheckerVisitor::visit(ast::membList* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();

    // iterate over varDecl nodes in membList
    for (auto n = dynamic_cast<ast::varDecl*>(node->child()); n != nullptr; n = dynamic_cast<ast::varDecl*>(n->next())) {
        auto entry = n->symbolTableEntry();

        // ignore any varDecl nodes that aren't being used (multiply declared, etc.)
        if (!entry || !entry->parentTable()) {
            continue;
        }

        // iterate over every superclass' symbol table for this class
        for (const auto& super : table->parentEntry()->supers()) {
            // check if the symbol is also defined in the super's symbol table
            if ((*super->link())[entry->name()]) {
                errors_->emplace_back(SemanticErrorType::SHADOWED_VARIABLE, dynamic_cast<ast::id*>(n->child(1))->token(), SemanticErrorLevel::WARN);
                n->marked = true;
            }
        }
    }
}

}}
