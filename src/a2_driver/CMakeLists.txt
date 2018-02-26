# target name
set(TARGET a2_driver)

# enable C++11
set(CMAKE_CXX_STANDARD 11)

# source files
set(SOURCE_FILES
        main.cpp)

# define target
add_executable(${TARGET} ${SOURCE_FILES})
target_link_libraries(${TARGET} moonshine)

configure_file(${CMAKE_SOURCE_DIR}/res/grammar.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/res/first.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/res/follow.txt ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/res/table.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COPYONLY)

#add_custom_command(TARGET ${TARGET} PRE_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${TARGET}>)

