cmake_minimum_required (VERSION 3.16)

add_library (vma INTERFACE)
target_include_directories (vma INTERFACE "include")
target_compile_features (vma INTERFACE cxx_std_17)
