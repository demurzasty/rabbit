cmake_minimum_required (VERSION 3.8.2)

add_executable (demo "src/main.cpp")
target_link_libraries (demo PUBLIC rabbit)

add_custom_command(
    TARGET demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            ${CMAKE_CURRENT_BINARY_DIR}/data
)
