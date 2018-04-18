# target name
set(TARGET moonshine)

# enable C++11
set(CMAKE_CXX_STANDARD 11)

# header files
set(HEADER_FILES
        Error.h
        Visitor.h
        lexer/nfa/State.h
        lexer/nfa/Atom.h
        lexer/dfa/DFA.h
        lexer/dfa/DFASimulator.h
        lexer/TokenType.h
        lexer/ParseError.h
        lexer/Token.h
        syntax/Grammar.h
        syntax/Parser.h
        syntax/Node.h
        syntax/ast_nodes.h
        syntax/ParseError.h
        semantic/TypeCheckerVisitor.h
        semantic/SymbolTableCreatorVisitor.h
        semantic/SymbolTable.h
        semantic/Type.h
        semantic/SemanticError.h
        semantic/SymbolTableClassDeclLinkerVisitor.h
        semantic/SymbolTableLinkerVisitor.h
        semantic/InheritanceResolverVisitor.h
        semantic/ShadowedSymbolCheckerVisitor.h
        code/MemorySizeComputerVisitor.h
        code/StackCodeGeneratorVisitor.h
        )

# source files
set(SOURCE_FILES
        Visitor.cpp
        lexer/Lexer.cpp
        lexer/nfa/NFA.cpp
        lexer/nfa/State.cpp
        lexer/nfa/Atom.cpp
        lexer/dfa/DFA.cpp
        lexer/dfa/DFASimulator.cpp
        lexer/TokenType.cpp
        syntax/Grammar.cpp
        syntax/Parser.cpp
        syntax/Node.cpp
        semantic/TypeCheckerVisitor.cpp
        semantic/SymbolTableCreatorVisitor.cpp
        semantic/SymbolTable.cpp
        semantic/Type.cpp
        semantic/SymbolTableClassDeclLinkerVisitor.cpp
        semantic/SymbolTableLinkerVisitor.cpp
        semantic/InheritanceResolverVisitor.cpp
        semantic/ShadowedSymbolCheckerVisitor.cpp
        code/MemorySizeComputerVisitor.cpp
        code/StackCodeGeneratorVisitor.cpp
        )

# define target
include_directories(${ROOT_INCLUDE_DIR})
add_library(${TARGET} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

# link additional libs
target_link_libraries(${TARGET} json)