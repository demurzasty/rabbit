cmake_minimum_required (VERSION 3.16)

add_library (entt INTERFACE)
target_include_directories (entt INTERFACE "include")
target_compile_features (entt INTERFACE cxx_std_17)
target_precompile_headers (entt INTERFACE <entt/entt.hpp>)
