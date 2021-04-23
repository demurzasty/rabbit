#pragma once

#include <rabbit/graphics/fwd.hpp>

#include <volk.h>

namespace rb {
    struct utils_vulkan {
        static VkBufferUsageFlags buffer_usage_flags(const buffer_type type);

        static VkFormat format(const texture_format& format);
    };
}