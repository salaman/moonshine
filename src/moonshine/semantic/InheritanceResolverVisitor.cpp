#include "moonshine/semantic/InheritanceResolverVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace semantic {

void InheritanceResolverVisitor::visit(ast::inherList* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();

    // find symbol table entry for this class
    auto classEntry = node->parent()->symbolTableEntry();

    // iterate over id nodes in inherList
    for (auto n = dynamic_cast<ast::id*>(node->child()); n != nullptr; n = dynamic_cast<ast::id*>(n->next())) {
        auto entry = (*table)[n->token()->value];

        if (!entry) {
            errors_->emplace_back(SemanticErrorType::UNDECLARED_VARIABLE, n->token()); // TODO
            continue;
        }

        classEntry->addSuper(entry);
    }
}

void InheritanceResolverVisitor::visit(ast::membList* node)
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
