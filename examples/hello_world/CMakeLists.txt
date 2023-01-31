cmake_minimum_required (VERSION 3.8.2)

add_executable (hello_world "src/main.cpp")
target_link_libraries (hello_world PUBLIC rabbit)

add_custom_command(
    TARGET hello_world POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/Data
            ${CMAKE_CURRENT_BINARY_DIR}/Data
)
