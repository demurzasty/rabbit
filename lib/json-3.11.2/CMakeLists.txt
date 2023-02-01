cmake_minimum_required (VERSION 3.16)

add_library (json INTERFACE)
target_include_directories (json INTERFACE "include")
target_compile_features (json INTERFACE cxx_std_17)
target_precompile_headers (json INTERFACE <nlohmann/json.hpp>)
