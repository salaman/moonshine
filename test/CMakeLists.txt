# target name
set(TARGET tests)

# enable C++14
set(CMAKE_CXX_STANDARD 14)

set(TEST_SOURCES
        # Sources here...
        main.cpp
        test_lexer.cpp
        )

add_executable(${TARGET} ${TEST_SOURCES})
target_link_libraries(${TARGET} Catch moonshine)

# CTest integration
#set(PARSE_CATCH_TESTS_VERBOSE ON)
#set(PARSE_CATCH_TESTS_ADD_TO_CONFIGURE_DEPENDS ON)
#include(ParseAndAddCatchTests)
#ParseAndAddCatchTests(${TARGET})