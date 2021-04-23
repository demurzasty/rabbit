cmake_minimum_required (VERSION 3.8.2)

add_library (json INTERFACE)
target_include_directories (json INTERFACE "include")
target_compile_features (json INTERFACE cxx_std_17)
