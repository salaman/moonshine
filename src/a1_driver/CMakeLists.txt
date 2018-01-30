# target name
set(TARGET a1_driver)

# enable C++11
set(CMAKE_CXX_STANDARD 11)

# source files
set(SOURCE_FILES
        main.cpp)

# define target
add_executable(${TARGET} ${SOURCE_FILES})
target_link_libraries(${TARGET} moonshine)