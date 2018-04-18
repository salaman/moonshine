#include <moonshine/lexer/Lexer.h>
#include <moonshine/lexer/Token.h>
#include <moonshine/lexer/TokenType.h>
#include <moonshine/syntax/Grammar.h>
#include <moonshine/syntax/Parser.h>
#include <moonshine/semantic/SemanticError.h>
#include <moonshine/semantic/InheritanceResolverVisitor.h>
#include <moonshine/semantic/SymbolTableCreatorVisitor.h>
#include <moonshine/semantic/SymbolTableClassDeclLinkerVisitor.h>
#include <moonshine/semantic/SymbolTableLinkerVisitor.h>
#include <moonshine/semantic/ShadowedSymbolCheckerVisitor.h>
#include <moonshine/semantic/TypeCheckerVisitor.h>
#include <moonshine/code/MemorySizeComputerVisitor.h>
#include <moonshine/code/StackCodeGeneratorVisitor.h>

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

    std::ifstream inputStream(argv[1]); // use sample file for lexing
    //std::istringstream inputStream("program { int a; float b; };"); // use string for lexing
    //std::istream& inputStream = std::cin; // use cin for lexing

    // derivation output (A2)

    std::ofstream tokenOutput("tokens.txt", std::ios::trunc); // use file
    //std::ofstream tokenOutput = &std::cout; // use stdout
    //std::ofstream tokenOutput; // disable output


    std::ofstream derivationOutput("derivation.txt", std::ios::trunc); // use file
    //std::ofstream derivationOutput = &std::cout; // use stdout
    //std::ofstream derivationOutput; // disable output

    // AST graphviz output

    std::ofstream astOutput("ast.dot", std::ios::trunc); // use file
    //std::ofstream astOutput; // disable output

    // symbol table output

    std::ofstream tableOutput("table.txt", std::ios::trunc); // use file
    //std::ostream& tableOutput = std::cout; // use stdout
    //std::ofstream tableOutput; // disable output

    // semantic error output

    //std::ofstream& errorOutput = tableOutput; // use file
    //std::ostream& errorOutput = std::cout; // use stdout
    std::ostream& errorOutput = std::cerr; // use stderr
    //std::ofstream errorOutput; // disable output

    // program output

    std::ofstream programOutput("program.m", std::ios::trunc); // use file
    //std::ostream& programOutput = std::cout;  // use stdout
    //std::ofstream programOutput; // disable output

    /*
     * DRIVER
     */

    std::vector<Error> errors;

    Lexer lex;
    syntax::Grammar grammar("grammar.txt", "table.json", "first.txt", "follow.txt");
    syntax::Parser parser(grammar);
    //parser.setAnsi(false); // set to true for color output

    lex.startLexing(&inputStream, &tokenOutput);

    std::unique_ptr<ast::Node> astRoot;
    astRoot = parser.parse(&lex, &derivationOutput); // disable output

    // output lexer errors
    for (const auto& e : lex.getErrors()) {
        std::ostringstream ss;
        e.print(ss);
        errors.emplace_back(e.position, ss.str());
    }

    // output parser errors
    for (const auto& e : parser.getErrors()) {
        std::ostringstream ss;
        e.print(ss);
        errors.emplace_back(e.token ? e.token->position : 0, ss.str());
    }

    if (astRoot) {
        std::vector<semantic::SemanticError> semanticErrors;
        std::vector<std::unique_ptr<Visitor>> visitors;
        visitors.emplace_back(new semantic::SymbolTableCreatorVisitor());
        visitors.emplace_back(new semantic::SymbolTableClassDeclLinkerVisitor());
        visitors.emplace_back(new semantic::SymbolTableLinkerVisitor());
        visitors.emplace_back(new semantic::InheritanceResolverVisitor());
        visitors.emplace_back(new semantic::ShadowedSymbolCheckerVisitor());
        visitors.emplace_back(new semantic::TypeCheckerVisitor());

        // run semantic check visitors
        for (auto& v : visitors) {
            v->setErrorContainer(&semanticErrors);
            astRoot->accept(v.get());
        }

        visitors.clear();

        std::ostringstream dataStream;

        if (errors.empty() && std::find_if(semanticErrors.begin(), semanticErrors.end(),
                         [](const semantic::SemanticError& e) { return e.level == semantic::SemanticErrorLevel::ERROR; }) == semanticErrors.end()) {
            visitors.emplace_back(new code::MemorySizeComputerVisitor());
            visitors.emplace_back(new code::StackCodeGeneratorVisitor(dataStream, programOutput));
            std::cout << "Code saved to program.m." << std::endl;
        } else {
            std::cout << "Code generation was surpressed because errors were found." << std::endl;
        }

        // run code generation visitors
        for (auto& v : visitors) {
            v->setErrorContainer(&semanticErrors);
            astRoot->accept(v.get());
        }

        programOutput << std::endl << dataStream.str();

        // print symbol tables
        astRoot->symbolTable()->print(tableOutput, "");

        // output semantic errors
        for (const auto& e : semanticErrors) {
            std::ostringstream ss;
            e.print(ss);
            errors.emplace_back(e.token ? e.token->position : 0, ss.str());
        }

        // print the AST tree graphviz output
        astRoot->graphviz(astOutput);
    }

    std::sort(errors.begin(), errors.end(), [](const Error& a, const Error& b) {
        return a.position < b.position;
    });

    for (const auto& e : errors) {
        errorOutput << e.message << std::endl;
    }

    return astRoot == nullptr;
}
