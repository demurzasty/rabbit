#pragma once 

#include <rabbit/graphics/renderer.hpp>
#include <rabbit/core/arena.hpp>

#include <volk.h>
#include <vma/vk_mem_alloc.h>

#include <vector>
#include <queue>

namespace rb {
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


        VkBuffer canvas_vertex_buffer;
        VmaAllocation canvas_vertex_buffer_allocation;
        unsigned int canvas_vertex_buffer_offset = 0;

        VkBuffer canvas_index_buffer;
        VmaAllocation canvas_index_buffer_allocation;
        unsigned int canvas_index_buffer_offset = 0;


        VkDescriptorSetLayout main_descriptor_set_layout;
        VkDescriptorPool main_descriptor_pool;
        VkDescriptorSet main_descriptor_set;

        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;


        arena<texture_data> textures;
        std::queue<texture_data> textures_to_delete;

        std::vector<draw_data> draw_commands;
    };
}
