#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/Token.h>
#include <moonshine/lexer/TokenType.h>
#include <moonshine/syntax/Grammar.h>
#include <moonshine/syntax/Parser.h>
#include <moonshine/semantic/TypeCheckVisitor.h>
#include <moonshine/semantic/SymbolTableCreatorVisitor.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <memory>

using namespace moonshine;

int main(int argc, const char** argv)
{
    Lexer lex;
    syntax::Grammar grammar("grammar.txt",  "table.json", "first.txt", "follow.txt");
    syntax::Parser parser(grammar);

    // use string for lexing
    std::string input = "program { int a; a = 1 + 2; };";
    std::istringstream stream(input);
    lex.startLexing(&stream);

    // use stdin for lexing
    //lex.startLexing(&std::cin);

    //parser.setAnsi(false); // set to true for color output

    std::ofstream derivationOutput("derivation.txt", std::ios::trunc);
    std::unique_ptr<ast::Node> astRoot;

    //astRoot = parser.parse(&lex, &derivationOutput); // file output
    //astRoot = parser.parse(&lex, &std::cout); // stdout output (supports colors!)
    astRoot = parser.parse(&lex, nullptr); // disable output

    // output errors
    for (const auto& e : parser.getErrors()) {
        switch (e.type) {
            case syntax::ParseErrorType::E_UNEXPECTED_TOKEN:
                derivationOutput << "error: unexpected token " << e.token->value << " at position " << e.token->position << std::endl;
            case syntax::ParseErrorType::E_UNEXPECTED_EOF:
                derivationOutput << "error: unexpected end of file reached" << std::endl;
                break;
            default:
                break;
        }
    }

    // print the AST tree graphviz output
    if (astRoot) {
        std::ofstream astOutput("ast.dot", std::ios::trunc);
        astRoot->graphviz(astOutput);
    }

    astRoot->print(&std::cout); std::cout << std::endl;

    auto visitor = std::make_shared<semantic::SymbolTableCreatorVisitor>();

    astRoot->accept(visitor);

    astRoot->symbolTable()->print(std::cout);

    return astRoot == nullptr;
}
