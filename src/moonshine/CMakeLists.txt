# target name
set(TARGET moonshine)

# enable C++11
set(CMAKE_CXX_STANDARD 11)

# header files
set(HEADER_FILES
        lexer/nfa/State.h
        lexer/nfa/Atom.h
        lexer/dfa/DFA.h
        lexer/dfa/DFASimulator.h
        lexer/TokenType.h
        lexer/ParseError.h
        lexer/Token.h
        )

# source files
set(SOURCE_FILES
        lexer/Lexer.cpp
        lexer/nfa/NFA.cpp
        lexer/nfa/State.cpp
        lexer/nfa/Atom.cpp
        lexer/dfa/DFA.cpp
        lexer/dfa/DFASimulator.cpp
        lexer/TokenType.cpp
        syntax/Grammar.cpp
        syntax/Grammar.h
        syntax/Parser.cpp
        syntax/Parser.h
        syntax/Node.cpp
        syntax/Node.h
        syntax/ast_nodes.h
        syntax/ParseError.h
        semantic/Visitor.cpp
        semantic/Visitor.h
        semantic/TypeCheckVisitor.h
        semantic/TypeCheckVisitor.cpp
        semantic/SymbolTableCreatorVisitor.h
        semantic/SymbolTableCreatorVisitor.cpp
        semantic/SymbolTable.cpp
        semantic/SymbolTable.h
        semantic/Type.cpp
        semantic/Type.h
        semantic/SemanticError.h
        semantic/SymbolTableLinkerVisitor.cpp
        semantic/SymbolTableLinkerVisitor.h
        semantic/InheritanceResolverVisitor.cpp
        semantic/InheritanceResolverVisitor.h
        semantic/DeclarationCheckerVisitor.h
        semantic/DeclarationCheckerVisitor.cpp
        )

# define target
include_directories(${ROOT_INCLUDE_DIR})
add_library(${TARGET} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

# link additional libs
target_link_libraries(${TARGET} json)