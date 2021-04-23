cmake_minimum_required (VERSION 3.8.2)

add_library (entt INTERFACE)
target_include_directories (entt INTERFACE "include")
target_compile_features (entt INTERFACE cxx_std_17)
target_compile_definitions (entt INTERFACE ENTT_ID_TYPE=std::size_t)
