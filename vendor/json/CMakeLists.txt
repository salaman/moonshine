cmake_minimum_required(VERSION 3.0)

project(json)

set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(json INTERFACE)
target_include_directories(json INTERFACE ${CATCH_INCLUDE_DIR})