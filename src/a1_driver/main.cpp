#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/Token.h>
#include <moonshine/lexer/TokenType.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>

using namespace moonshine;

int main(int argc, const char** argv)
{
    bool atocc = false;

    if (argc > 1 && strcmp(argv[1], "atocc") == 0) {
        atocc = true;
    }

    moonshine::Lexer lex;

    //std::istringstream stream("int Utility::findMin(int array[100]){");
    //lex.startLexing(&stream);
    lex.startLexing(&std::cin);

    moonshine::Token* token = nullptr;

    while ((token = lex.getNextToken()) != nullptr) {
        if (atocc) {
            std::cout << token->name() << ' ';
        } else {
            std::cout << token->name() << " \"" << token->value << "\" " << token->position << std::endl;
        }

        delete token;
    }

    std::cout << std::endl;

    for (const auto& e : lex.getErrors()) {
        switch (e.type) {
            case ParseErrorType::E_INVALID_CHARACTERS:
                std::cerr << "Invalid characters";
                break;
            case ParseErrorType::E_UNTERMINATED_COMMENT:
                std::cerr << "Unterminated comment";
                break;
            default:
                break;
        }

        std::cerr << ": \"" << e.value << "\" @ " << e.position << std::endl;
    }

    return 0;
}
