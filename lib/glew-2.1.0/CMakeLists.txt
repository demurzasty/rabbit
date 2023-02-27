cmake_minimum_required (VERSION 3.8.2)

add_library (GLEW STATIC "src/glew.c")
target_include_directories (GLEW PUBLIC "include")
target_compile_definitions (GLEW PUBLIC GLEW_STATIC)
