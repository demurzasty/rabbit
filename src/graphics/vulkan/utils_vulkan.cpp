#include "utils_vulkan.hpp"

#include <rabbit/core/config.hpp>

using namespace rb;

VkCommandBuffer utils_vulkan::begin_single_time_commands(VkDevice device, VkCommandPool command_pool) {
    // Create temporary buffer
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool = command_pool;
    allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &allocate_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info;
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = nullptr;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    begin_info.pInheritanceInfo = nullptr;

    // Begin registering commands
    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void utils_vulkan::end_single_time_commands(VkDevice device, VkQueue graphics_queue, VkCommandPool command_pool, VkCommandBuffer command_buffer) {
    // End registering commands
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

VkFormat utils_vulkan::format(const texture_format& format) {
    switch (format) {
        case texture_format::r8: return VK_FORMAT_R8_UNORM;
        case texture_format::rg8: return VK_FORMAT_R8G8_UNORM;
        case texture_format::rgba8: return VK_FORMAT_R8G8B8A8_UNORM;
        case texture_format::bgra8: return VK_FORMAT_B8G8R8A8_UNORM;
        case texture_format::d24s8: return VK_FORMAT_D24_UNORM_S8_UINT;
        case texture_format::undefined: return VK_FORMAT_UNDEFINED;
    }

    RB_ASSERT(false, "Failed to map Vulkan format");
    return VK_FORMAT_UNDEFINED;
}

VkFormat utils_vulkan::format(const vertex_format& format) {
    switch (format.type) {
        case vertex_type::floating_point:
            switch (format.components) {
                case 1: return VK_FORMAT_R32_SFLOAT;
                case 2: return VK_FORMAT_R32G32_SFLOAT;
                case 3: return VK_FORMAT_R32G32B32_SFLOAT;
                case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            break;
        case vertex_type::integer:
            switch (format.components) {
                case 1: return VK_FORMAT_R32_UINT;
                case 2: return VK_FORMAT_R32G32_UINT;
                case 3: return VK_FORMAT_R32G32B32_UINT;
                case 4: return VK_FORMAT_R32G32B32A32_UINT;
            }
            break;
    }

    RB_ASSERT(false, "Failed to map Vulkan format");
    return VK_FORMAT_UNDEFINED;
}

VkPrimitiveTopology utils_vulkan::topology(rb::topology topology) {
    switch (topology) {
        case topology::lines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case topology::line_strip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case topology::triangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case topology::triangle_strip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    }

    RB_ASSERT(false, "Failed to map Vulkan primitive topology");
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

VkImageType utils_vulkan::image_type(texture_type texture_type) {
    switch (texture_type) {
        case texture_type::texture_2d:
        case texture_type::texture_cube:
            return VK_IMAGE_TYPE_2D;
    }

    RB_ASSERT(false, "Failed to map Vulkan image type");
    return VK_IMAGE_TYPE_2D;
}

VkImageViewType utils_vulkan::image_view_type(texture_type texture_type) {
    switch (texture_type) {
        case texture_type::texture_2d:
            return VK_IMAGE_VIEW_TYPE_2D;
        case texture_type::texture_cube:
            return VK_IMAGE_VIEW_TYPE_CUBE;
    }

    RB_ASSERT(false, "Failed to map Vulkan image view type");
    return VK_IMAGE_VIEW_TYPE_2D;
}

VkImageCreateFlags utils_vulkan::image_create_flags(texture_type texture_type) {
    VkImageCreateFlags flags{ 0 };

    if (texture_type == texture_type::texture_cube) {
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    return flags;
}

VkFilter utils_vulkan::filter(texture_filter texture_filter) {
    switch (texture_filter) {
        case texture_filter::nearest:
            return VK_FILTER_NEAREST;
        case texture_filter::linear:
            return VK_FILTER_LINEAR;
    }

    RB_ASSERT(false, "Failed to map Vulkan filter");
    return VK_FILTER_NEAREST;
}

VkSamplerAddressMode utils_vulkan::sampler_address_mode(texture_wrap texture_wrap) {
    switch (texture_wrap) {
        case texture_wrap::repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case texture_wrap::clamp:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }

    RB_ASSERT(false, "Failed to map Vulkan sampler address mode");
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkIndexType utils_vulkan::index_type(rb::index_type index_type) {
    switch (index_type) {
        case index_type::uint16:
            return VK_INDEX_TYPE_UINT16;
        case index_type::uint32:
            return VK_INDEX_TYPE_UINT32;
    }

    RB_ASSERT(false, "Failed to map Vulkan index type");
    return VK_INDEX_TYPE_UINT32;
}
