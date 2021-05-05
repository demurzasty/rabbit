#include "mesh_vulkan.hpp"
#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>

#include <cstring>

using namespace rb;

mesh_vulkan::mesh_vulkan(const mesh_desc& desc)
    : mesh(desc) {
}
