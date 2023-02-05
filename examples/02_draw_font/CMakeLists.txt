cmake_minimum_required (VERSION 3.8.2)

add_executable (draw_font "src/main.cpp")
target_link_libraries (draw_font PUBLIC rabbit)

add_custom_command(
    TARGET draw_font POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            ${CMAKE_CURRENT_BINARY_DIR}/data
)
