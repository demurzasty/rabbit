#include "material_vulkan.hpp"

#include <rabbit/core/config.hpp>
#include <rabbit/math/vec2.hpp>
using namespace rb;

material_vulkan::material_vulkan(VkDevice device,
    VmaAllocator allocator,
    const material_desc& desc)
    : material(desc)
    , _device(device)
    , _allocator(allocator) {
    _create_uniform_buffer(desc);
}

material_vulkan::~material_vulkan() {
    vmaDestroyBuffer(_allocator, _uniform_buffer, _uniform_buffer_allocation);
}

VkBuffer material_vulkan::buffer() const {
    return _uniform_buffer;
}

void material_vulkan::_create_uniform_buffer(const material_desc& desc) {
    VkBufferCreateInfo uniform_buffer_info;
    uniform_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uniform_buffer_info.pNext = nullptr;
    uniform_buffer_info.flags = 0;
    uniform_buffer_info.size = sizeof(material_data);
    uniform_buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    uniform_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uniform_buffer_info.queueFamilyIndexCount = 0;
    uniform_buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo uniform_buffer_allocation_info;
    uniform_buffer_allocation_info.flags = 0;
    uniform_buffer_allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    uniform_buffer_allocation_info.requiredFlags = 0;
    uniform_buffer_allocation_info.preferredFlags = 0;
    uniform_buffer_allocation_info.memoryTypeBits = 0;
    uniform_buffer_allocation_info.pool = VK_NULL_HANDLE;
    uniform_buffer_allocation_info.pUserData = nullptr;

    RB_MAYBE_UNUSED auto result = vmaCreateBuffer(_allocator,
        &uniform_buffer_info,
        &uniform_buffer_allocation_info,
        &_uniform_buffer,
        &_uniform_buffer_allocation,
        nullptr);
    RB_ASSERT(result == VK_SUCCESS, "Cannot create Vulkan uniform buffer");

    material_data material_data;
    material_data.base_color = desc.base_color;
    material_data.roughness = desc.roughness;
    material_data.metallic = desc.metallic;

    void* data;
    result = vmaMapMemory(_allocator, _uniform_buffer_allocation, &data);
    RB_ASSERT(result == VK_SUCCESS, "Failed to map uniform buffer memory");

    memcpy(data, &material_data, sizeof(material_data));

    vmaUnmapMemory(_allocator, _uniform_buffer_allocation);
}
