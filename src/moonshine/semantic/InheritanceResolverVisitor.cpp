#include "moonshine/semantic/InheritanceResolverVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>
//#include <iostream>

namespace moonshine { namespace semantic {

void InheritanceResolverVisitor::visit(ast::classList* node)
{
    Visitor::visit(node);

    auto table = node->closestSymbolTable();

    for (auto it = table->begin(); it != table->end(); ++it) {
        std::shared_ptr<SymbolTableEntry> classEntry = it->second;

        if (classEntry->kind() != SymbolTableEntryKind::CLASS) {
            continue;
        }

        std::vector<std::string> supers;
        std::map<std::string, bool> leaf;
        std::map<std::string, std::string> tree;
        supers.emplace_back(classEntry->name());
        tree[classEntry->name()] = classEntry->name();
        //std::cout << classEntry->name() << std::endl;

        for (std::vector<std::string>::size_type i = 0; i < supers.size(); ++i) {
            auto currentSuper = supers[i];
            //std::cout << ">> " << currentSuper << std::endl;
            leaf[currentSuper] = true;

            for (const auto& super : (*table)[currentSuper]->supers()) {
                //std::cout << ">>>> " << super->name() << std::endl;
                leaf[currentSuper] = false;

                if (std::find(supers.begin(), supers.end(), super->name()) != supers.end()) {
                    if (!leaf[super->name()]) {
                        std::string root = currentSuper;
                        while (tree[root] != classEntry->name()) {
                            root = tree[root];
                        }

                        //std::cout << ">>>> *** CIRCULAR! " << super->name() << std::endl;
                        //std::cout << ">>>> *** removing " << root << std::endl;
                        classEntry->removeSuper(root);

                        std::shared_ptr<Token> token;
                        for (auto classDecl = node->child(); classDecl != nullptr; classDecl = classDecl->next()) {
                            if (classDecl->symbolTableEntry()->name() == classEntry->name()) {
                                for (auto id = dynamic_cast<ast::id*>(classDecl->child(1)->child()); id != nullptr; id = dynamic_cast<ast::id*>(id->next())) {
                                    if (id->token()->value == root) {
                                        token = id->token();
                                        break;
                                    }
                                }

                                if (token) {
                                    break;
                                }
                            }
                        }
                        errors_->emplace_back(SemanticErrorType::CIRCULAR_INHERITANCE, token);

                        break;
                    }
                } else {
                    tree[super->name()] = currentSuper;
                    supers.emplace_back(super->name());
                }
            }
        }
    }
}

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

        // check that the class this is referring to exists
        if (!entry) {
            errors_->emplace_back(SemanticErrorType::UNDECLARED_CLASS, n->token());
            continue;
        }

        // check that this symbol is a class
        if (entry->kind() != SymbolTableEntryKind::CLASS) {
            errors_->emplace_back(SemanticErrorType::INVALID_CLASS, n->token());
            continue;
        }

        // check that this isn't a duplicate entry in the super list
        auto nt = std::find_if(classEntry->supers().begin(), classEntry->supers().end(), [&entry](const std::shared_ptr<SymbolTableEntry>& super) {
            return super->name() == entry->name();
        });

        if (nt != classEntry->supers().end()) {
            errors_->emplace_back(SemanticErrorType::DUPLICATE_SUPER, n->token());
            continue;
        }

        // OK!
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

void InheritanceResolverVisitor::visit(ast::classDecl* node)
{
    Visitor::visit(node);

    // find closest symbol table
    auto table = node->closestSymbolTable();
    auto classEntry = node->symbolTableEntry();

    if (!classEntry) {
        return;
    }

    //std::cout << classEntry->name() << std::endl;

    for (const auto& entry : *classEntry->link()) {
        auto type = dynamic_cast<VariableType*>(entry.second->type());

        if (entry.second->kind() != SymbolTableEntryKind::VARIABLE || type->type != Type::CLASS) {
            continue;
        }

        //std::cout << ">> " << type->className << std::endl;

        std::vector<std::string> members;
        members.emplace_back(type->className);

        for (std::vector<std::string>::size_type i = 0; i < members.size(); ++i) {
            auto currentMember = members[i];

            for (const auto& e : *(*table)[currentMember]->link()) {
                auto memberType = dynamic_cast<VariableType*>(e.second->type());

                if (e.second->kind() != SymbolTableEntryKind::VARIABLE || memberType->type != Type::CLASS) {
                    continue;
                }

                auto member = memberType->className;
                //std::cout << ">>>> " << member << std::endl;

                if (member == classEntry->name()) {
                    //std::cout << ">>>> *** CIRCULAR! " << member << std::endl;

                    std::shared_ptr<Token> token;
                    for (auto varDecl = node->child(2)->child(); varDecl != nullptr; varDecl = varDecl->next()) {
                        if (dynamic_cast<ast::id*>(varDecl->child(1))->token()->value == entry.first) {
                            token = dynamic_cast<ast::id*>(varDecl->child(1))->token();
                            break;
                        }
                    }
                    errors_->emplace_back(SemanticErrorType::CIRCULAR_MEMBER, token);

                    break;
                } else if (std::find(members.begin(), members.end(), member) == members.end()) {
                    members.emplace_back(member);
                }
            }
        }
    }
}

}}
