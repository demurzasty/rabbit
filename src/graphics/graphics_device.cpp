#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/core/config.hpp>

using namespace rb;

graphics_device::graphics_device(settings& settings, window& window)
    : _window(window) {
}

window& graphics_device::associated_window() {
    return _window;
}
