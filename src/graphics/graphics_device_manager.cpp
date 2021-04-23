#include <rabbit/graphics/graphics_device_manager.hpp>

#if RB_VULKAN
#   include "vulkan/graphics_device_vulkan.hpp"
    using graphics_device_impl = rb::graphics_device_vulkan;
#endif

using namespace rb;

std::shared_ptr<graphics_device> graphics_device_manager::create(const graphics_device_desc& desc) {
    return _graphics_devices.emplace_back(std::make_shared<graphics_device_impl>(desc));
}
