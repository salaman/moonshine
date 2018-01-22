cmake_minimum_required(VERSION 3.2)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

project(comp442)

# modules
include(CTest)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(Warnings)

# external dependencies
add_subdirectory(vendor/catch EXCLUDE_FROM_ALL)

# project sources
add_subdirectory(src)