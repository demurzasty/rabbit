cmake_minimum_required (VERSION 3.16)

add_library (imgui STATIC
    "imgui.cpp"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
    "misc/cpp/imgui_stdlib.cpp"
)
target_include_directories (imgui PUBLIC "." "misc/cpp")
target_compile_features (imgui PUBLIC cxx_std_17)
