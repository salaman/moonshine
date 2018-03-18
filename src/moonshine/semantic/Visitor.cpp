#include "moonshine/semantic/Visitor.h"

namespace moonshine { namespace semantic {

void Visitor::setErrorContainer(std::vector<SemanticError>* errors)
{
    errors_ = errors;
}

}}
