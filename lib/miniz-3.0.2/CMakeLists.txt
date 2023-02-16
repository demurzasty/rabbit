cmake_minimum_required (VERSION 3.8.2)

add_library (miniz STATIC "miniz.c")
target_include_directories (miniz PUBLIC ".")
