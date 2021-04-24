#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/graphics/buffer.hpp>
#include <rabbit/graphics/texture.hpp>
#include <rabbit/graphics/shader.hpp>
#include <rabbit/graphics/vertex.hpp>

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

VkFormat utils_vulkan::format(const vertex_format& format) {
    switch (format.type) {
        case vertex_format_type::floating_point:
            switch (format.components) {
                case 1: return VK_FORMAT_R32_SFLOAT;
                case 2: return VK_FORMAT_R32G32_SFLOAT;
                case 3: return VK_FORMAT_R32G32B32_SFLOAT;
                case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            break;
        case vertex_format_type::integer:
            switch (format.components) {
                case 1: return VK_FORMAT_R32_UINT;
                case 2: return VK_FORMAT_R32G32_UINT;
                case 3: return VK_FORMAT_R32G32B32_UINT;
                case 4: return VK_FORMAT_R32G32B32A32_UINT;
            }
            break;
    }

    RB_ASSERT(false, "Failed to map Vulkan format");
    return VK_FORMAT_UNDEFINED;
}

VkDescriptorType utils_vulkan::descriptor_type(const shader_binding_type& type) {
    switch (type) {
        case shader_binding_type::texture:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case shader_binding_type::uniform_buffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }

    RB_ASSERT(false, "Failed to map Vulkan descriptor type");
    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkShaderStageFlags utils_vulkan::stage_flags(const std::uint32_t flags) {
    VkShaderStageFlags bitfield = 0;

    if (flags & shader_stage_flags::vertex) {
        bitfield |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (flags & shader_stage_flags::fragment) {
        bitfield |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    return bitfield;
}