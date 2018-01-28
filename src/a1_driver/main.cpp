#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/Token.h>
#include <moonshine/lexer/TokenType.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

//using namespace moonshine;

int main()
{
    moonshine::Lexer lex;

    std::istringstream stream("__abc ; 1.2000");
    //std::istringstream stream("abc");

    //std::cout << stream.tellg() << std::endl;
    //std::cout << (char) stream.get() << std::endl;
    //std::cout << stream.tellg() << std::endl;
    //std::cout << (char) stream.get() << std::endl;
    //std::cout << stream.tellg() << std::endl;
    //std::cout << (char) stream.get() << std::endl;
    //std::cout << stream.tellg() << std::endl;
    //std::cout << (char) stream.get() << std::endl;
    //std::cout << stream.eof() << std::endl;

    lex.startLexing(&stream);

    moonshine::Token* token = nullptr;

    while ((token = lex.getNextToken()) != nullptr) {
        std::cout << token->name() << " \"" << token->value << "\" " << token->position << std::endl;

        delete token;
    }

    //// simulate various test strings
    //std::vector<std::string> tests = {
    //    "",
    //    "abc",
    //    "ab_c",
    //    "0abc",
    //    "0",
    //    "1",
    //    "123",
    //    "123.",
    //    "123.0",
    //    "=",
    //    "==",
    //    "a",
    //    "an",
    //    "and",
    //    "not",
    //};
    //
    //for (const auto& s : tests) {
    //    dfa::DFASimulator simulator(dfa);
    //
    //    std::for_each(s.cbegin(), s.cend(), [&simulator](const char& c) {
    //        simulator.move(c);
    //    });
    //
    //    std::cout << s << ": " << (simulator.accepted() ? simulator.token() : "n/a") << std::endl;
    //}

    return 0;
}