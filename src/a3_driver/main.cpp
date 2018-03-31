#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/Token.h>
#include <moonshine/lexer/TokenType.h>
#include <moonshine/syntax/Grammar.h>
#include <moonshine/syntax/Parser.h>
#include <moonshine/semantic/SemanticError.h>
#include <moonshine/semantic/InheritanceResolverVisitor.h>
#include <moonshine/semantic/SymbolTableCreatorVisitor.h>
#include <moonshine/semantic/SymbolTableLinkerVisitor.h>
#include <moonshine/semantic/DeclarationCheckerVisitor.h>
#include <moonshine/semantic/TypeCheckVisitor.h>

#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <memory>
#include <vector>
#include <istream>

using namespace moonshine;

int main(int argc, const char** argv)
{

    /*
     * CONFIGURATION
     */

    // input

    std::ifstream inputStream("sample.txt"); // use sample file for lexing
    //std::istringstream inputStream("program { int a; float b; };"); // use string for lexing
    //std::istream& inputStream = std::cin; // use cin for lexing

    // derivation output (A2)

    //std::ofstream derivationOutput("derivation.txt", std::ios::trunc); // use file
    //std::ofstream derivationOutput = &std::cout; // use stdout
    std::ofstream derivationOutput; // disable output

    // AST graphviz output

    std::ofstream astOutput("ast.dot", std::ios::trunc); // use file
    //std::ofstream astOutput; // disable output

    // symbol table output

    //std::ofstream tableOutput("table.txt", std::ios::trunc); // use file
    std::ostream& tableOutput = std::cout; // use stdout
    //std::ofstream tableOutput; // disable output

    // semantic error output

    //std::ofstream& errorOutput = tableOutput; // use file
    //std::ostream& errorOutput = std::cout; // use stdout
    std::ostream& errorOutput = std::cerr; // use stderr
    //std::ofstream errorOutput; // disable output

    /*
     * DRIVER
     */

    Lexer lex;
    syntax::Grammar grammar("grammar.txt", "table.json", "first.txt", "follow.txt");
    syntax::Parser parser(grammar);
    //parser.setAnsi(false); // set to true for color output

    lex.startLexing(&inputStream);

    std::unique_ptr<ast::Node> astRoot;
    astRoot = parser.parse(&lex, &derivationOutput); // disable output

    // output parser errors
    for (const auto& e : parser.getErrors()) {
        switch (e.type) {
            case syntax::ParseErrorType::E_UNEXPECTED_TOKEN:
                errorOutput << "error: unexpected token " << e.token->value << " at position " << e.token->position << std::endl;
            case syntax::ParseErrorType::E_UNEXPECTED_EOF:
                errorOutput << "error: unexpected end of file reached" << std::endl;
                break;
            default:
                break;
        }
    }

    if (astRoot) {
        std::vector<semantic::SemanticError> errors;
        std::vector<std::unique_ptr<semantic::Visitor>> visitors;
        visitors.emplace_back(new semantic::SymbolTableCreatorVisitor());
        visitors.emplace_back(new semantic::SymbolTableLinkerVisitor());
        visitors.emplace_back(new semantic::InheritanceResolverVisitor());
        visitors.emplace_back(new semantic::DeclarationCheckerVisitor());
        visitors.emplace_back(new semantic::TypeCheckVisitor());

        // run visitors
        for (auto& v : visitors) {
            v->setErrorContainer(&errors);
            astRoot->accept(v.get());
        }

        // print symbol tables
        astRoot->symbolTable()->print(tableOutput, "");

        // output semantic errors
        for (const auto& e : errors) {
            switch (e.level) {
                case semantic::SemanticErrorLevel::ERROR:
                    errorOutput << "[ERROR] ";
                    break;
                case semantic::SemanticErrorLevel::WARN:
                    errorOutput << "[WARN] ";
                    break;
            }

            switch (e.type) {
                case semantic::SemanticErrorType::UNDECLARED_VARIABLE:
                    errorOutput << "Undeclared variable";
                    break;
                case semantic::SemanticErrorType::INCOMPATIBLE_TYPE:
                    errorOutput << "Incompatible type";
                    break;
                case semantic::SemanticErrorType::INVALID_SUBSCRIPT_TYPE:
                    errorOutput << "Invalid subscript";
                    break;
                case semantic::SemanticErrorType::INVALID_DIMENSION_COUNT:
                    errorOutput << "Invalid dimension count";
                    break;
                case semantic::SemanticErrorType::REDECLARED_SYMBOL:
                    errorOutput << "Redeclared symbol";
                    break;
                case semantic::SemanticErrorType::UNDECLARED_FUNCTION:
                    errorOutput << "Undeclared function";
                    break;
                case semantic::SemanticErrorType::INVALID_VARIABLE:
                    errorOutput << "Invalid variable";
                    break;
                case semantic::SemanticErrorType::UNDECLARED_MEMBER_VARIABLE:
                    errorOutput << "Undeclared member variable";
                    break;
                case semantic::SemanticErrorType::UNDECLARED_MEMBER_FUNCTION:
                    errorOutput << "Undeclared member function";
                    break;
                case semantic::SemanticErrorType::INVALID_FUNCTION:
                    errorOutput << "Invalid function";
                    break;
                case semantic::SemanticErrorType::UNDEFINED_FUNCTION:
                    errorOutput << "Undefined function";
                    break;
                case semantic::SemanticErrorType::REDEFINED_FUNCTION:
                    errorOutput << "Redefined function";
                    break;
                case semantic::SemanticErrorType::INCOMPATIBLE_RETURN_TYPE:
                    errorOutput << "Incompatible return type";
                    break;
                case semantic::SemanticErrorType::SHADOWED_VARIABLE:
                    errorOutput << "Shadowed variable";
                    break;
                case semantic::SemanticErrorType::MISSING_RETURN:
                    errorOutput << "Missing return";
                    break;
                case semantic::SemanticErrorType::INVALID_RETURN:
                    errorOutput << "Invalid return";
                    break;
            }

            if (e.token) {
                errorOutput << " for " << e.token->value << " (" << TokenName[e.token->type] << ") at position " << e.token->position;
            }
            
            errorOutput << std::endl;
        }

        // print the AST tree graphviz output
        astRoot->graphviz(astOutput);
    }

    return astRoot == nullptr;
}
