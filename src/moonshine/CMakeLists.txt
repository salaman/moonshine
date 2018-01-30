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
        )

# define target
include_directories(${ROOT_INCLUDE_DIR})
add_library(${TARGET} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")