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

    syntax::Grammar grammar("grammar.txt",  "table.json");
    std::cout << std::endl;

    std::cout << "Parsing: " << input << std::endl;
    std::cout << std::endl;
    syntax::Parser parser(grammar);
    bool success = parser.parse(&lex);

    return !success;
}
