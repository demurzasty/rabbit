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
    for (auto& installation : builder._installations) {
        installation(_injector);
    }
}

int application::run() {
    auto& window = _injector.get<rb::window>();
    auto& graphics_device = _injector.get<rb::graphics_device>();

    auto shader = graphics_device.create_shader(builtin_shaders::get(builtin_shader::forward));

    camera_data camera_data;
    auto camera_buffer = create_uniform_buffer(graphics_device, camera_data);

    local_data local_data;
    auto local_buffer = create_uniform_buffer(graphics_device, local_data);

    material_data material_data;
    auto material_buffer = create_uniform_buffer(graphics_device, material_data);

    resource_heap_desc resource_heap_desc;
    resource_heap_desc.shader = shader;
    resource_heap_desc.resources = {
        { 0, camera_buffer },
        { 1, local_buffer },
        { 2, material_buffer },
    };
    auto resource_heap = graphics_device.create_resource_heap(resource_heap_desc);

    const vertex vertices[] = {
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
        { { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } }
    };

    buffer_desc buffer_desc;
    buffer_desc.type = buffer_type::vertex;
    buffer_desc.size = sizeof(vertices);
    buffer_desc.stride = sizeof(vertex);
    buffer_desc.data = vertices;
    auto vertex_buffer = graphics_device.create_buffer(buffer_desc);

    auto command_buffer = graphics_device.create_command_buffer();

    while (window.is_open()) {
        window.poll_events();

        command_buffer->begin();

        command_buffer->update_buffer(local_buffer, &local_data, 0, sizeof(local_data));

        command_buffer->begin_render_pass(graphics_device);

        command_buffer->set_shader(shader);

        command_buffer->set_resource_heap(resource_heap);

        command_buffer->set_vertex_buffer(vertex_buffer);

        command_buffer->draw(3, 1, 0, 0);

        command_buffer->end_render_pass();

        command_buffer->end();

        graphics_device.submit(command_buffer);

        graphics_device.present();
    }

    return 0;
}
