#include <catch/catch.hpp>

#include <moonshine/lexer/Lexer.h>
#include <moonshine/syntax/Parser.h>
#include <moonshine/semantic/SymbolTable.h>
#include <moonshine/semantic/SymbolTableCreatorVisitor.h>
#include <moonshine/semantic/SymbolTableLinkerVisitor.h>
#include <moonshine/semantic/TypeCheckerVisitor.h>

#include <sstream>
#include <memory>

using namespace moonshine;

#define TEST_AST(INPUT, SEM) \
TEST_CASE(INPUT, "[syntax]") { \
    Lexer lex; \
    syntax::Grammar grammar("grammar.txt",  "table.json", "first.txt", "follow.txt"); \
    \
    std::istringstream stream((INPUT)); \
    lex.startLexing(&stream, nullptr); \
    syntax::Parser parser(grammar); \
    \
    std::unique_ptr<ast::Node> astRoot; \
    astRoot = parser.parse(&lex, nullptr); \
    \
    REQUIRE(astRoot != nullptr); \
    std::ostringstream oss; \
    astRoot->print(&oss); \
    REQUIRE(oss.str() == (SEM)); \
}

#define REQUIRE_FUNCTION(NAME, X) \
{ \
    std::shared_ptr<semantic::SymbolTableEntry> entry = (*table)[#NAME]; \
    REQUIRE(entry); \
    REQUIRE(entry->kind() == semantic::SymbolTableEntryKind::FUNCTION); \
    std::shared_ptr<semantic::SymbolTable> table = entry->link(); \
    REQUIRE(table); \
    X \
}

#define REQUIRE_ENTRY(NAME) \
{ \
    std::shared_ptr<semantic::SymbolTableEntry> entry = (*table)[#NAME]; \
    REQUIRE(entry); \
    std::shared_ptr<semantic::SymbolTable> table = entry->link(); \
    REQUIRE(table); \
}
