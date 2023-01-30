cmake_minimum_required (VERSION 3.16)

add_library (gsl INTERFACE)
target_include_directories (gsl INTERFACE "include")
target_compile_features (gsl INTERFACE cxx_std_17)
target_precompile_headers (gsl INTERFACE <gsl/gsl>)
