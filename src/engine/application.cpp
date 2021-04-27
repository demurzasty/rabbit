#include <rabbit/engine/application.hpp>
#include <rabbit/engine/builder.hpp>
#include <rabbit/platform/window.hpp>
#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/graphics/builtin_shaders.hpp>
#include <rabbit/graphics/shader.hpp>
#include <rabbit/graphics/buffer.hpp>
#include <rabbit/graphics/command_buffer.hpp>
#include <rabbit/graphics/resource_heap.hpp>
#include <rabbit/math/vec2.hpp>
#include <rabbit/math/vec3.hpp>
#include <rabbit/math/mat4.hpp>
#include <rabbit/engine/system.hpp>
#include <rabbit/engine/entity.hpp>
#include <rabbit/engine/settings.hpp>

#include <chrono>

using namespace rb;

struct camera_data {
    mat4f projection{ mat4f::perspective(0.6108652382f, 1.7777f, 0.1f, 100.0f) };
    mat4f view{ mat4f::translation({ 0.0f, 0.0f, -10.0f }) };
};

struct local_data {
    mat4f world{ mat4f::identity() };
};

struct material_data {
    vec3f base_color{ 0.2f, 0.5f, 0.8f };
    float roughness{ 0.8f };
    float metallic{ 0.0f };
};

struct vertex {
    vec3f position;
    vec2f texcoord;
    vec3f normal;
};

template<typename T>
std::shared_ptr<buffer> create_uniform_buffer(graphics_device& graphics_device, const T& data) {
    buffer_desc desc;
    desc.type = buffer_type::uniform;
    desc.size = sizeof(T);
    desc.stride = desc.size;
    desc.data = &data;
    return graphics_device.create_buffer(desc);
}

application::application(const builder& builder) {
    for (auto& [priority, installation] : builder._installations) {
        installation(_injector);
    }

    for (auto& system_factory : builder._system_factories) {
        _systems.push_back(system_factory(_injector));
    }
}

int application::run() {
    auto& settings = _injector.get<rb::settings>();
    auto& window = _injector.get<rb::window>();
    auto& graphics_device = _injector.get<rb::graphics_device>();

    registry registry;
    for (auto& system : _systems) {
        system->initialize(registry);
    }

    auto last_time = std::chrono::steady_clock::now();
    auto current_time = last_time;
    auto accumulation_time = 0.0f;

    while (window.is_open()) {
        window.poll_events();

        current_time = std::chrono::steady_clock::now();
        const auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(current_time - last_time).count();
        last_time = current_time;

        for (auto& system : _systems) {
            system->update(registry, elapsed_time);
        }

        accumulation_time += elapsed_time;

        while (accumulation_time >= settings.fixed_time) {
            for (auto& system : _systems) {
                system->fixed_update(registry, settings.fixed_time);
            }

            accumulation_time -= settings.fixed_time;
        }

        for (auto& system : _systems) {
            system->draw(registry, graphics_device);
        }

        graphics_device.present();
    }

    return 0;
}
