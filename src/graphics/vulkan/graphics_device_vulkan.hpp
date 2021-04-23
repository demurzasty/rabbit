#pragma once

#include <rabbit/graphics/graphics_device.hpp>

namespace rb {
    class graphics_device_vulkan : public graphics_device {
    public:
        graphics_device_vulkan(const graphics_device_desc& desc);

        ~graphics_device_vulkan();
    };
}
