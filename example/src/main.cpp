#include <rabbit/platform/window_manager.hpp>
#include <rabbit/platform/window.hpp>
#include <rabbit/graphics/graphics_device_manager.hpp>
#include <rabbit/graphics/graphics_device.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    window_manager window_manager;
    auto window = window_manager.create({});

    window->size();

    graphics_device_manager graphics_device_manager;
    auto graphics_device = graphics_device_manager.create({ window });

    while (window->is_open()) {
        window->poll_events();

        graphics_device->present();
    }
}
