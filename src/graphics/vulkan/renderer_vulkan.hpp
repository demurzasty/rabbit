#pragma once 

#include <rabbit/graphics/renderer.hpp>
#include <rabbit/core/arena.hpp>

#include <volk.h>
#include <vma/vk_mem_alloc.h>

#include <vector>

namespace rb {
    struct texture_data {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView image_view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        uvec2 size = { 0, 0 };
        pixel_format format = pixel_format::undefined;
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

        std::uint32_t image_index = 0;

        arena<texture_data> textures;
    };
}
