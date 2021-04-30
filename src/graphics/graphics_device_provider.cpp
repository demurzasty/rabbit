#include <rabbit/graphics/graphics_device_provider.hpp>

#if RB_VULKAN
#   include "vulkan/graphics_device_vulkan.hpp"
    using graphics_device_impl = rb::graphics_device_vulkan;
#endif

using namespace rb;

std::shared_ptr<graphics_device> graphics_device_provider::operator()(injector& injector) const {
    return injector.resolve<graphics_device_impl>();
}
