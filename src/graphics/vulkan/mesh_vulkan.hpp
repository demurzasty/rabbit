#pragma once

#include <rabbit/graphics/mesh.hpp>

#include <volk.h>
#include <vk_mem_alloc.h>

namespace rb {
    class mesh_vulkan : public mesh {
    public:
        mesh_vulkan(VkDevice device, VmaAllocator allocator, const mesh_desc& desc);

        ~mesh_vulkan();

        VkBuffer vertex_buffer() const;

        VkBuffer index_buffer() const;

    private:
        void _create_vertex_buffer(const mesh_desc& desc);

        void _create_index_buffer(const mesh_desc& desc);

    private:
        VkDevice _device;
        VmaAllocator _allocator;

        VkBuffer _vertex_buffer;
        VmaAllocation _vertex_buffer_allocation;

        VkBuffer _index_buffer{ VK_NULL_HANDLE };
        VmaAllocation _index_buffer_allocation;
    };
}
