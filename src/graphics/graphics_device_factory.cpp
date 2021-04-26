#include <rabbit/graphics/graphics_device_factory.hpp>
#include <rabbit/platform/window.hpp>
#include <rabbit/core/injector.hpp>

#if RB_VULKAN
#   include "vulkan/graphics_device_vulkan.hpp"
    using graphics_device_impl = rb::graphics_device_vulkan;
#endif

using namespace rb;

std::shared_ptr<graphics_device> graphics_device_factory::operator()(injector& injector) const {
    return injector.resolve<graphics_device_impl>();
}
