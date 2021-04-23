#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/core/config.hpp>

using namespace rb;

graphics_device::graphics_device(const graphics_device_desc& desc)
    : _window(desc.window) {
    RB_ASSERT(_window, "No window has been provided");
}

const std::shared_ptr<window>& graphics_device::associated_window() const {
    return _window;
}
