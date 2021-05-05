#pragma once

#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/core/settings.hpp>
#include <rabbit/platform/window.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

// TODO: Replace std::vector.
#include <vector>

namespace rb {
    class graphics_device_vulkan : public graphics_device {
    public:
        graphics_device_vulkan(settings& settings, window& window);

        ~graphics_device_vulkan();

        std::shared_ptr<renderer> create_renderer() override;

        std::shared_ptr<command_buffer> create_command_buffer() override;

        std::shared_ptr<buffer> create_buffer(const buffer_desc& desc) override;

        std::shared_ptr<texture> create_texture(const texture_desc& desc) override;

        std::shared_ptr<shader> create_shader(const shader_desc& desc) override;

        std::shared_ptr<mesh> create_mesh(const mesh_desc& desc) override;

        std::shared_ptr<material> create_material(const material_desc& desc) override;

        void submit(VkCommandBuffer command_buffer, VkFence fence);

        void present() override;

        VkDevice device() const;

        VkCommandPool command_pool() const;

        VmaAllocator allocator() const;

        VkRenderPass render_pass() const;

        VkFramebuffer framebuffer() const;

        VkExtent2D swapchain_extent() const;

    private:
        void _initialize_volk(const settings& settings);

        void _create_instance(const settings& settings);

        void _choose_physical_device(const settings& settings);

        void _create_surface(const settings& settings, window& window);

        void _create_device(const settings& settings);

        void _create_allocator(const settings& settings);

        void _query_surface(const settings& settings);

        void _create_swapchain(const settings& settings, window& window);

        void _create_command_pool(const settings& settings);

        void _create_synchronization_objects(const settings& settings);

    private:
        VkInstance _instance;
        VkPhysicalDevice _physical_device;
        VkSurfaceKHR _surface;
        VkDevice _device;

        std::uint32_t _graphics_family;
        std::uint32_t _present_family;
        VkQueue _graphics_queue;
        VkQueue _present_queue;

        VmaAllocator _allocator;
        VkSurfaceFormatKHR _surface_format;
        VkExtent2D _swapchain_extent;

        VkSwapchainKHR _swapchain;
        VkPresentModeKHR _present_mode;

        VkImage _depth_image;
        VmaAllocation _depth_image_allocation;
        VkImageView _depth_image_view;

        std::vector<VkImage> _images;
        std::vector<VkImageView> _image_views;
        std::vector<VkFramebuffer> _framebuffers;

        VkRenderPass _render_pass;

        VkCommandPool _command_pool;

        VkSemaphore _render_semaphore;
        VkSemaphore _present_semaphore;

        std::uint32_t _image_index{ 0 };
    };
}
