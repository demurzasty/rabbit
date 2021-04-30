#pragma once

#include <rabbit/graphics/material.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace rb {
    class material_vulkan : public material {
    public:
        material_vulkan(VkDevice device, VmaAllocator allocator, const material_desc& desc);

        ~material_vulkan();

        VkBuffer buffer() const;

    private:
        struct material_data {
            vec3f base_color;
            float roughness;
            float metallic;
        };

    private:
        VkDevice _device;
        VmaAllocator _allocator;

        VkBuffer _buffer;
        VmaAllocation _buffer_allocation;
    };
}
