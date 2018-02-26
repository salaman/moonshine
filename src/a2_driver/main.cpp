#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/Token.h>
#include <moonshine/lexer/TokenType.h>
#include <moonshine/syntax/Grammar.h>
#include <moonshine/syntax/Parser.h>

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

    //std::string input = "program { int idx; };";
    //std::string input = "program { id = id().id[id][id].id(); };";
    std::string input = "program { int idx[5]; idx[1] = id(3+4); };";

    std::istringstream stream(input);
    lex.startLexing(&stream);

    syntax::Grammar grammar("grammar.txt",  "table.json", "first.txt", "follow.txt");
    std::cout << std::endl;

    std::cout << "Parsing: " << input << std::endl;
    std::cout << std::endl;

    std::ofstream astOutput("ast.dot", std::ios::trunc);

    syntax::Parser parser(grammar);

    std::unique_ptr<ast::Node> astRoot;

    astRoot = parser.parse(&lex);

    if (astRoot) {
        astRoot->graphviz(astOutput);
    }

    return astRoot == nullptr;
}
