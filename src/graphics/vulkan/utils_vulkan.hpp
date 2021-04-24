#pragma once

#include <rabbit/graphics/fwd.hpp>

#include <volk.h>

namespace rb {
    struct utils_vulkan {
        static VkBufferUsageFlags buffer_usage_flags(const buffer_type type);

        static VkFormat format(const texture_format& format);

        static VkFormat format(const vertex_format& format);

        static VkDescriptorType descriptor_type(const shader_binding_type& type);

        static VkShaderStageFlags stage_flags(const std::uint32_t flags);
    };
}