# target name
set(TARGET a1_driver)

# enable C++14
set(CMAKE_CXX_STANDARD 14)

# source files
set(SOURCE_FILES
        main.cpp)

# define target
add_executable(${TARGET} ${SOURCE_FILES})
target_set_warnings(${TARGET} ENABLE ALL AS_ERROR ALL DISABLE Annoying)
target_link_libraries(${TARGET} moonshine)
