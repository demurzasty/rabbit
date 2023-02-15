cmake_minimum_required (VERSION 3.8.2)

add_executable (input "src/main.cpp")
target_link_libraries (input PUBLIC rabbit)
