#pragma once

#include <rabbit/core/config.hpp>
#include <rabbit/graphics/buffer.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace rb {
    class buffer_vulkan : public buffer {
    public:
        buffer_vulkan(VkDevice device, VmaAllocator allocator, const buffer_desc& desc);

        ~buffer_vulkan();

        void update(const void* data) override;

        VkBuffer buffer() const;

    private:
        VkDevice _device;
        VmaAllocator _allocator;

        VkBuffer _buffer;
        VmaAllocation _allocation;
    };
}