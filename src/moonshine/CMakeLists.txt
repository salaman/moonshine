# target name
set(TARGET moonshine)

# enable C++14
set(CMAKE_CXX_STANDARD 14)

# header files
set(HEADER_FILES
        lexer/Lexer.h)

# source files
set(SOURCE_FILES
        lexer/Lexer.cpp
        lexer/Token.cpp
        lexer/Token.h
        lexer/nfa/NFA.cpp
        lexer/nfa/NFA.h
        lexer/nfa/State.cpp
        lexer/nfa/State.h
#        lexer/nfa/ICharacter.cpp
#        lexer/nfa/ICharacter.h
#        lexer/TokenBuilder.cpp
#        lexer/TokenBuilder.h
#        lexer/nfa/NFABuilder.cpp
#        lexer/nfa/NFABuilder.h
        lexer/nfa/Atom.cpp
        lexer/nfa/Atom.h
        lexer/dfa/DFA.cpp
        lexer/dfa/DFA.h
        lexer/dfa/CombinedState.cpp
        lexer/dfa/CombinedState.h lexer/dfa/DFASimulator.cpp lexer/dfa/DFASimulator.h)

# define target
include_directories(${ROOT_INCLUDE_DIR})
add_library(${TARGET} STATIC ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_set_warnings(${TARGET} ENABLE ALL AS_ERROR ALL DISABLE Annoying)