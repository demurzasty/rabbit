#pragma once 

#include <rabbit/graphics/renderer.hpp>
#include <rabbit/core/arena.hpp>

#include <volk.h>
#include <vma/vk_mem_alloc.h>

#include <vector>

namespace rb {
    struct alignas(16) gpu_global_data {
        int sprite_count = 0;
    };

    struct alignas(16) gpu_texture_data {
        int sampler_id = -1;
    };

    struct alignas(16) gpu_sprite_data {
        vec2 position = vec2::zero();
        float rotation = 0.0f;
        vec2 scale = vec2::one();
        int visible = 1;
        int texture_id = -1;
    };

    struct texture_data {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        uvec2 size = { 0, 0 };
        pixel_format format = pixel_format::undefined;
    };

    struct draw_data {
        int texture_index = -1;
        unsigned int index_offset = 0;
        unsigned int index_count = 0;
        unsigned int vertex_offset = 0;
    };

    struct renderer::data {
        VkInstance instance;
        VkSurfaceKHR surface;
        VkPhysicalDevice physical_device;
        VkPhysicalDeviceProperties physical_device_properties;
        uint32_t graphics_family;
        uint32_t present_family;
        VkDevice device;
        VkQueue graphics_queue;
        VkQueue present_queue;
        VmaAllocator allocator;

        VkSurfaceFormatKHR surface_format;
        VkExtent2D swapchain_extent;
        VkSwapchainKHR swapchain;
        VkPresentModeKHR present_mode;

        std::vector<VkImage> screen_images;
        std::vector<VkImageView> screen_image_views;

        VkRenderPass screen_render_pass;
        std::vector<VkFramebuffer> screen_framebuffers;

        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> command_buffers;
        std::vector<VkFence> fences;

        VkSemaphore render_semaphore;
        VkSemaphore present_semaphore;

        unsigned int image_index = 0;

        VkBuffer global_buffer;
        VmaAllocation global_buffer_allocation;

        VkBuffer canvas_vertex_buffer;
        VmaAllocation canvas_vertex_buffer_allocation;
        unsigned int canvas_vertex_buffer_offset = 0;

        VkBuffer canvas_index_buffer;
        VmaAllocation canvas_index_buffer_allocation;
        unsigned int canvas_index_buffer_offset = 0;

        VkBuffer texture_buffer;
        VmaAllocation texture_buffer_allocation;

        VkBuffer sprite_buffer;
        VmaAllocation sprite_buffer_allocation;

        VkBuffer sprite_draw_buffer;
        VmaAllocation sprite_draw_buffer_allocation;

        VkDescriptorSetLayout main_descriptor_set_layout;
        VkDescriptorPool main_descriptor_pool;
        VkDescriptorSet main_descriptor_set;

        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;

        gpu_global_data gpu_global_data;
        arena<void> gpu_samplers;
        arena<gpu_sprite_data> gpu_sprites;

        arena<texture_data> textures;

        std::vector<draw_data> draw_commands;
    };
}
