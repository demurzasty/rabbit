#include "buffer_vulkan.hpp"
#include "utils_vulkan.hpp"

#include <cstring>

using namespace rb;

buffer_vulkan::buffer_vulkan(VkDevice device, VmaAllocator allocator, const buffer_desc& desc)
    : rb::buffer(desc)
    , _device(device)
    , _allocator(allocator) {
    VkBufferCreateInfo buffer_info;
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.pNext = nullptr;
    buffer_info.flags = 0;
    buffer_info.size = desc.size;
    buffer_info.usage = utils_vulkan::buffer_usage_flags(desc.type) | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_info.queueFamilyIndexCount = 0;
    buffer_info.pQueueFamilyIndices = nullptr;

    VmaAllocationCreateInfo allocation_info = {};
	allocation_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    RB_MAYBE_UNUSED auto result = vmaCreateBuffer(allocator, &buffer_info, &allocation_info, &_buffer, &_allocation, nullptr);

    if (desc.data) {
        update(desc.data);
    }
}

buffer_vulkan::~buffer_vulkan() {
    vmaDestroyBuffer(_allocator, _buffer, _allocation);
}

void buffer_vulkan::update(const void* data) {
    void* ptr;
    RB_MAYBE_UNUSED auto result = vmaMapMemory(_allocator, _allocation, &ptr);
    RB_ASSERT(result == VK_SUCCESS, "Failed to map memory");

    std::memcpy(ptr, data, size());

    vmaUnmapMemory(_allocator, _allocation);
}

VkBuffer buffer_vulkan::buffer() const {
    return _buffer;
}
