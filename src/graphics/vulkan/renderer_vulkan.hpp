#pragma once

#include <rabbit/graphics/renderer.hpp>
#include <rabbit/core/span.hpp>
#include <rabbit/math/mat4.hpp>

#include "graphics_device_vulkan.hpp"

#include <volk.h>
#include <vk_mem_alloc.h>

#include <unordered_map>

namespace rb {
    class renderer_vulkan : public renderer {
        struct camera_data {
            mat4f proj;
            mat4f view;
        };

        struct local_data {
            mat4f world;
        };

        struct material_data {
            vec3f base_color;
            float roughness;
            float metallic;
        };

        struct entity_local_data {
            local_data local_data;

            VkBuffer local_buffer{ VK_NULL_HANDLE };
            VmaAllocation local_buffer_allocation{ VK_NULL_HANDLE };

            VkDescriptorPool descriptor_pool{ VK_NULL_HANDLE };
            VkDescriptorSet descriptor_set{ VK_NULL_HANDLE };
        };

    public:
        renderer_vulkan(graphics_device_vulkan& graphics_device);

        ~renderer_vulkan();

        void render(registry& registry) override;

    private:
        void _begin();

        void _end();

        void _begin_main_render_pass();

        void _end_main_render_pass();

        void _create_command_buffer();

        void _create_camera_buffer();

        void _create_local_buffer(entity_local_data& entity_local_data);

        void _create_descriptor_set(entity_local_data& entity_local_data);

        void _update_descriptor_set(entity_local_data& entity_local_data, const std::shared_ptr<material>& material);

        void _create_forward_pipeline();

        VkShaderModule _create_shader_module(const span<const std::uint32_t>& bytecode);

    private:
        graphics_device_vulkan& _graphics_device;

        VkDevice _device;
        VkCommandPool _command_pool;
        VmaAllocator _allocator;
        VkRenderPass _main_render_pass;

        VkCommandBuffer _command_buffer;
        VkFence _fence;

        VkDescriptorSetLayout _forward_descriptor_layout;
        VkPipelineLayout _forward_pipeline_layout;
        VkPipeline _forward_pipeline;

        VkBuffer _camera_buffer;
        VmaAllocation _camera_buffer_allocation;

        std::shared_ptr<texture> _white_texture;

        std::unordered_map<entity, entity_local_data> _entity_local_data;
    };
}
