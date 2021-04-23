#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/graphics/buffer.hpp>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

using namespace rb;

VkBufferUsageFlags utils_vulkan::buffer_usage_flags(const buffer_type type) {
    switch (type) {
        case buffer_type::vertex: return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case buffer_type::index: return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case buffer_type::uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default: RB_ASSERT(false, "Missing mapping"); return 0;
    }
}
