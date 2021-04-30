#include "mesh_vulkan.hpp"
#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>

#include <cstring>

using namespace rb;

mesh_vulkan::mesh_vulkan(VkDevice device, VmaAllocator allocator, const mesh_desc& desc)
    : mesh(desc)
    , _device(device)
    , _allocator(allocator) {
    _create_vertex_buffer(desc);

    if (desc.index_data) {
        _create_index_buffer(desc);
    }
}

mesh_vulkan::~mesh_vulkan() {
    if (_index_buffer) {
        vmaDestroyBuffer(_allocator, _index_buffer, _index_buffer_allocation);
    }

    vmaDestroyBuffer(_allocator, _vertex_buffer, _vertex_buffer_allocation);
}

VkBuffer mesh_vulkan::vertex_buffer() const {
    return _vertex_buffer;
}

VkBuffer mesh_vulkan::index_buffer() const {
    return _index_buffer;
}

void mesh_vulkan::_create_vertex_buffer(const mesh_desc& desc) {
    std::size_t vertex_stride{ 0 };
    for (const auto& vertex_element : desc.vertex_layout) {
        vertex_stride += vertex_element.format.size;
    }

    VkBufferCreateInfo vertex_buffer_info;
    vertex_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_info.pNext = nullptr;
    vertex_buffer_info.flags = 0;
    vertex_buffer_info.size = desc.vertex_count * vertex_stride;
    vertex_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    vertex_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vertex_buffer_info.queueFamilyIndexCount = 0;
    vertex_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo vertex_buffer_allocation_info;
    vertex_buffer_allocation_info.flags = 0;
    vertex_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    vertex_buffer_allocation_info.requiredFlags = 0;
    vertex_buffer_allocation_info.preferredFlags = 0;
    vertex_buffer_allocation_info.memoryTypeBits = 0;
    vertex_buffer_allocation_info.pool = VK_NULL_HANDLE;
    vertex_buffer_allocation_info.pUserData = nullptr;

    RB_MAYBE_UNUSED auto result = vmaCreateBuffer(_allocator,
        &vertex_buffer_info,
        &vertex_buffer_allocation_info,
        &_vertex_buffer,
        &_vertex_buffer_allocation,
        nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Cannot create Vulkan vertex buffer");

    void* data;
    result = vmaMapMemory(_allocator, _vertex_buffer_allocation, &data);
    RB_ASSERT(result == VK_SUCCESS, "Failed to map vertex buffer memory");

    memcpy(data, desc.vertex_data, vertex_buffer_info.size);

    vmaUnmapMemory(_allocator, _vertex_buffer_allocation);
}

void mesh_vulkan::_create_index_buffer(const mesh_desc& desc) {
    std::size_t index_stride{ desc.index_type == index_type::uint16 ? 2u : 4u };

    VkBufferCreateInfo index_buffer_info;
    index_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    index_buffer_info.pNext = nullptr;
    index_buffer_info.flags = 0;
    index_buffer_info.size = desc.index_count * index_stride;
    index_buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    index_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    index_buffer_info.queueFamilyIndexCount = 0;
    index_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo index_buffer_allocation_info;
    index_buffer_allocation_info.flags = 0;
    index_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    index_buffer_allocation_info.requiredFlags = 0;
    index_buffer_allocation_info.preferredFlags = 0;
    index_buffer_allocation_info.memoryTypeBits = 0;
    index_buffer_allocation_info.pool = VK_NULL_HANDLE;
    index_buffer_allocation_info.pUserData = nullptr;

    RB_MAYBE_UNUSED auto result = vmaCreateBuffer(_allocator,
        &index_buffer_info,
        &index_buffer_allocation_info,
        &_index_buffer,
        &_index_buffer_allocation,
        nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Cannot create Vulkan index buffer");

    void* data;
    result = vmaMapMemory(_allocator, _index_buffer_allocation, &data);
    RB_ASSERT(result == VK_SUCCESS, "Failed to map index buffer memory");

    memcpy(data, desc.index_data, index_buffer_info.size);

    vmaUnmapMemory(_allocator, _index_buffer_allocation);
}