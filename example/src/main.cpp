#include <rabbit/platform/window_manager.hpp>
#include <rabbit/platform/window.hpp>
#include <rabbit/graphics/graphics_device_manager.hpp>
#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/graphics/command_buffer.hpp>
#include <rabbit/graphics/builtin_shaders.hpp>
#include <rabbit/graphics/shader.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    window_manager window_manager;
    auto window = window_manager.create({});

    graphics_device_manager graphics_device_manager;
    auto graphics_device = graphics_device_manager.create({ window });

    auto shader = graphics_device->create_shader(builtin_shaders::get(builtin_shader::forward));

    auto command_buffer = graphics_device->create_command_buffer();

    while (window->is_open()) {
        window->poll_events();

        command_buffer->begin();

        command_buffer->begin_render_pass(graphics_device);

        command_buffer->set_shader(shader);

        command_buffer->end_render_pass();

        command_buffer->end();

        graphics_device->submit(command_buffer);

        graphics_device->present();
    }
}
