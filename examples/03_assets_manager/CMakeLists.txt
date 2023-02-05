cmake_minimum_required (VERSION 3.8.2)

add_executable (assets_manager "src/main.cpp")
target_link_libraries (assets_manager PUBLIC rabbit)

add_custom_command(
    TARGET assets_manager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            ${CMAKE_CURRENT_BINARY_DIR}/data
)
