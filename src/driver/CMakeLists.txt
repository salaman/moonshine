# target name
set(TARGET driver)

# enable C++11
set(CMAKE_CXX_STANDARD 11)

# source files
set(SOURCE_FILES
        main.cpp)

# define target
add_executable(${TARGET} ${SOURCE_FILES})
target_link_libraries(${TARGET} moonshine)

configure_file(${CMAKE_SOURCE_DIR}/res/grammar.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/grammar.txt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/res/first.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/first.txt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/res/follow.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/follow.txt COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/res/table.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/table.json COPYONLY)
