#include "moonshine/Visitor.h"

namespace moonshine {

void Visitor::setErrorContainer(std::vector<semantic::SemanticError>* errors)
{
    errors_ = errors;
}

}
