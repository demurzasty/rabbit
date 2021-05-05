#pragma once

#include <rabbit/graphics/material.hpp>
#include <rabbit/core/span.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace rb {
    class material_vulkan : public material {
    public:
        material_vulkan(VkDevice device, VmaAllocator allocator, const material_desc& desc);

        ~material_vulkan();

        VkBuffer buffer() const;

    private:
        void _create_uniform_buffer(const material_desc& desc);

    private:
        VkDevice _device;
        VmaAllocator _allocator;
        VkRenderPass _render_pass;

        VkBuffer _uniform_buffer;
        VmaAllocation _uniform_buffer_allocation;
    };
}
