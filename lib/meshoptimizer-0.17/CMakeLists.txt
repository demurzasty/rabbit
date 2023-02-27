cmake_minimum_required(VERSION 3.0)

add_library (meshoptimizer STATIC 
    "include/meshoptimizer.h"
    "src/allocator.cpp"
    "src/clusterizer.cpp"
    "src/indexcodec.cpp"
    "src/indexgenerator.cpp"
    "src/overdrawanalyzer.cpp"
    "src/overdrawoptimizer.cpp"
    "src/simplifier.cpp"
    "src/spatialorder.cpp"
    "src/stripifier.cpp"
    "src/vcacheanalyzer.cpp"
    "src/vcacheoptimizer.cpp"
    "src/vertexcodec.cpp"
    "src/vertexfilter.cpp"
    "src/vfetchanalyzer.cpp"
    "src/vfetchoptimizer.cpp"
)
target_include_directories (meshoptimizer PUBLIC "include")