#pragma once

#include <rabbit/graphics/shader.hpp>

#include <volk.h>

namespace rb {
    class shader_vulkan : public shader {
    public:
        shader_vulkan(VkDevice device,
            VkRenderPass render_pass,
            VkExtent2D swapchain_extent,
            const shader_desc& desc);

        ~shader_vulkan();

        VkDescriptorSetLayout descriptor_set_layout() const;

        VkPipelineLayout pipeline_layout() const;

        VkPipeline pipeline() const;

    private:
        void _create_descriptor_set_layout(const shader_desc& desc);

        void _create_pipeline_layout(const shader_desc& desc);

        VkShaderModule _create_shader_module(const span<const std::uint32_t>& bytecode);

        void _create_pipeline(const span<const VkPipelineShaderStageCreateInfo>& shader_stages,
            VkRenderPass render_pass,
            VkExtent2D swapchain_extent,
            const shader_desc& desc);

    private:
        VkDevice _device;

        VkDescriptorSetLayout _descriptor_set_layout;
        VkPipelineLayout _pipeline_layout;
        VkPipeline _pipeline;
    };
}