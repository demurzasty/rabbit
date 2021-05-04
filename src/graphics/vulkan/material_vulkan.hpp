#pragma once

#include <rabbit/graphics/material.hpp>
#include <rabbit/core/span.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace rb {
    class material_vulkan : public material {
    public:
        material_vulkan(VkDevice device, VmaAllocator allocator, VkRenderPass render_pass, const material_desc& desc);

        ~material_vulkan();

        VkBuffer buffer() const;

        VkDescriptorSetLayout descriptor_set_layout() const;

        VkPipelineLayout pipeline_layout() const;

        VkPipeline pipeline() const;

    private:
        void _create_uniform_buffer(const material_desc& desc);

        void _create_pipeline(const material_desc& desc);

        VkShaderModule _create_shader_module(const span<const std::uint32_t>& bytecode);

    private:
        struct material_data {
            vec3f base_color;
            float roughness;
            float metallic;
        };

    private:
        VkDevice _device;
        VmaAllocator _allocator;
        VkRenderPass _render_pass;

        VkBuffer _uniform_buffer;
        VmaAllocation _uniform_buffer_allocation;

        VkDescriptorSetLayout _descriptor_set_layout;
        VkPipelineLayout _pipeline_layout;
        VkPipeline _pipeline;
    };
}
