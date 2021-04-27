#pragma once

#include <rabbit/graphics/fwd.hpp>

#include <volk.h>

#include <cstdint>

namespace rb {
    struct utils_vulkan {
        static VkBufferUsageFlags buffer_usage_flags(const buffer_type type);

        static VkFormat format(const texture_format& format);

        static VkFormat format(const vertex_format& format);

        static VkDescriptorType descriptor_type(const shader_binding_type& type);

        static VkShaderStageFlags stage_flags(const std::uint32_t flags);

        static VkPrimitiveTopology topology(topology topology);

        static VkPolygonMode polygon_mode(polygon_mode polygon_mode);

        static VkCullModeFlags cull_mode(cull_mode cull_mode);

        static VkFrontFace front_face(front_face front_face);

        static VkCompareOp compare_operator(compare_operator compare_operator);

        static VkImageType image_type(texture_type texture_type);

        static VkImageViewType image_view_type(texture_type texture_type);

        static VkImageCreateFlags image_create_flags(texture_type texture_type);

        static VkFilter filter(texture_filter texture_filter);

        static VkSamplerAddressMode sampler_address_mode(texture_wrap texture_wrap);
    };
}