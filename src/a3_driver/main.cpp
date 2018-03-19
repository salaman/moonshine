#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/Token.h>
#include <moonshine/lexer/TokenType.h>
#include <moonshine/syntax/Grammar.h>
#include <moonshine/syntax/Parser.h>
#include <moonshine/semantic/SemanticError.h>
#include <moonshine/semantic/TypeCheckVisitor.h>
#include <moonshine/semantic/SymbolTableCreatorVisitor.h>
#include <moonshine/semantic/StatementBlockDeclVisitor.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <memory>
#include <vector>

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

    if (astRoot) {
        std::vector<semantic::SemanticError> errors;

        std::vector<std::unique_ptr<semantic::Visitor>> visitors;
        visitors.emplace_back(new semantic::SymbolTableCreatorVisitor());
        visitors.emplace_back(new semantic::StatementBlockDeclVisitor());
        visitors.emplace_back(new semantic::TypeCheckVisitor());

        for (auto& v : visitors) {
            v->setErrorContainer(&errors);
            astRoot->accept(v.get());
        }

        astRoot->symbolTable()->print(std::cout);

        for (const auto& e : errors) {
            switch (e.level) {
                case semantic::SemanticErrorLevel::ERROR:
                    std::cout << "[ERROR] ";
                    break;
                case semantic::SemanticErrorLevel::WARN:
                    std::cout << "[WARN] ";
                    break;
            }

            switch (e.type) {
                case semantic::SemanticErrorType::UNDECLARED_VARIABLE:
                    std::cout << "Undeclared variable" << std::endl;
                    break;
                case semantic::SemanticErrorType::INCOMPATIBLE_TYPE:
                    std::cout << "Incompatible type" << std::endl;
                    break;
                case semantic::SemanticErrorType::INVALID_SUBSCRIPT_TYPE:
                    std::cout << "Invalid subscript" << std::endl;
                    break;
                case semantic::SemanticErrorType::INVALID_DIMENSION_COUNT:
                    std::cout << "Invalid dimension count" << std::endl;
                    break;
                case semantic::SemanticErrorType::REDECLARED_SYMBOL:
                    std::cout << "Redeclared symbol" << std::endl;
                    break;
                case semantic::SemanticErrorType::UNDECLARED_FUNCTION:
                    std::cout << "Undeclared function" << std::endl;
                    break;
                case semantic::SemanticErrorType::INVALID_VARIABLE:
                    std::cout << "Invalid variable" << std::endl;
                    break;
                case semantic::SemanticErrorType::UNDECLARED_MEMBER_VARIABLE:
                    std::cout << "Undeclared member variable" << std::endl;
                    break;
                case semantic::SemanticErrorType::UNDECLARED_MEMBER_FUNCTION:
                    std::cout << "Undeclared member function" << std::endl;
                    break;
                case semantic::SemanticErrorType::INVALID_FUNCTION:
                    std::cout << "Invalid function" << std::endl;
                    break;
            }
        }

        // print the AST tree graphviz output
        std::ofstream astOutput("ast.dot", std::ios::trunc);
        astRoot->graphviz(astOutput);
    }

    return astRoot == nullptr;
}
