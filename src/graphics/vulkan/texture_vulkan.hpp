#pragma once

#include <rabbit/graphics/texture.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

#include <memory>

namespace rb {
    class texture_vulkan : public texture {
    public:
        texture_vulkan(VkDevice device, VkQueue graphics_queue, VkCommandPool command_pool, VmaAllocator allocator, const texture_desc& desc);

        ~texture_vulkan();

        VkImage image() const;

        VkImageView image_view() const;

        VkImageView target_image_view(std::size_t layer) const;

        VkSampler sampler() const;

    private:
        void _create_image(const texture_desc& desc);

        void _update_image(VkQueue graphics_queue, VkCommandPool command_pool, const texture_desc& desc);

        void _generate_mipmaps(VkQueue graphics_queue, VkCommandPool command_pool, const texture_desc& desc);

        void _create_image_view(const texture_desc& desc);

        void _create_framebuffer(const texture_desc& desc);

        void _create_sampler(const texture_desc& desc);

        void _create_render_pass(const texture_desc& desc);

        void _transition_image_layout(const texture_desc& desc, VkQueue graphics_queue, VkCommandPool command_pool, VkImageLayout old_layout, VkImageLayout new_layout);

    private:
        VkDevice _device;
        VmaAllocator _allocator;
        VmaAllocation _allocation;
        VkImage _image;
        VkImageView _image_view;
        VkSampler _sampler;
        VkRenderPass _render_pass{ VK_NULL_HANDLE };
        std::unique_ptr<VkImageView[]> _target_image_views;
        std::unique_ptr<VkFramebuffer[]> _framebuffers;
    };
}
