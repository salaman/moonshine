#include "moonshine/code/MemorySizeComputerVisitor.h"

#include "moonshine/lexer/TokenType.h"
#include "moonshine/semantic/Type.h"

#include <memory>
#include <string>
#include <utility>

namespace moonshine { namespace code {

void MemorySizeComputerVisitor::visit(ast::prog* node)
{
    Visitor::visit(node);
}

}}
