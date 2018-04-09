#include "moonshine/Visitor.h"

namespace moonshine {

void Visitor::setErrorContainer(std::vector<semantic::SemanticError>* errors)
{
    errors_ = errors;
}

void Visitor::next(ast::Node* node)
{
    for (auto n = node->child(); n != nullptr; n = n->next()) {
        n->accept(this);
    }
}

}
