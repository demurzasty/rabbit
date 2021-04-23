#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/graphics/buffer.hpp>
#include <rabbit/graphics/texture.hpp>

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

VkFormat utils_vulkan::format(const texture_format& format) {
    switch (format) {
        case texture_format::r8: return VK_FORMAT_R8_UNORM;
        case texture_format::rg8: return VK_FORMAT_R8G8_UNORM;
        case texture_format::rgba8: return VK_FORMAT_R8G8B8A8_UNORM;
        case texture_format::d24s8: return VK_FORMAT_D24_UNORM_S8_UINT;
    }

    RB_ASSERT(false, "Failed to map Vulkan format");
    return VK_FORMAT_UNDEFINED;
}
