
#pragma once

#include <rabbit/graphics/buffer.hpp>
#include <rabbit/graphics/texture.hpp>
#include <rabbit/graphics/mesh.hpp>
#include <rabbit/graphics/shader.hpp>

#include <volk.h>

#include <cstdint>

namespace rb {
    struct utils_vulkan {
        static VkCommandBuffer begin_single_time_commands(VkDevice device, VkCommandPool command_pool);

        static void end_single_time_commands(VkDevice device, VkQueue graphics_queue, VkCommandPool command_pool, VkCommandBuffer command_buffer);

        static VkBufferUsageFlags buffer_usage_flags(buffer_type type);

        static VkFormat format(texture_format format);

        static VkFormat format(const vertex_format& format);

        static VkPrimitiveTopology topology(topology topology);

        static VkImageType image_type(texture_type texture_type);

        static VkImageViewType image_view_type(texture_type texture_type);

        static VkImageCreateFlags image_create_flags(texture_type texture_type);

        static VkFilter filter(texture_filter texture_filter);

        static VkSamplerAddressMode sampler_address_mode(texture_wrap texture_wrap);

        static VkDescriptorType descriptor_type(const shader_binding_type& type);

        static VkShaderStageFlags stage_flags(const std::uint32_t flags);

        static VkPolygonMode polygon_mode(polygon_mode polygon_mode);

        static VkCullModeFlags cull_mode(cull_mode cull_mode);

        static VkFrontFace front_face(front_face front_face);

        static VkCompareOp compare_operator(compare_operator compare_operator);
    };
}
