cmake_minimum_required (VERSION 3.8.2)

add_library (rgbcx STATIC "src/rgbcx.cpp")
target_include_directories (rgbcx PUBLIC "include")
