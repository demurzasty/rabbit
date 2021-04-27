#include <rabbit/platform/window_factory.hpp>
#include <rabbit/graphics/graphics_device_factory.hpp>
#include <rabbit/core/injector.hpp>
#include <rabbit/engine/settings.hpp>
#include <rabbit/engine/application.hpp>
#include <rabbit/engine/builder.hpp>
#include <rabbit/engine/system.hpp>
#include <rabbit/asset/asset_manager.hpp>
#include <rabbit/loaders/texture_loader.hpp>
#include <rabbit/loaders/mesh_loader.hpp>
#include <rabbit/graphics/builtin_shaders.hpp>
#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/graphics/shader.hpp>
#include <rabbit/math/vec2.hpp>
#include <rabbit/math/vec3.hpp>
#include <rabbit/math/mat4.hpp>
#include <rabbit/graphics/resource_heap.hpp>
#include <rabbit/graphics/command_buffer.hpp>
#include <rabbit/graphics/mesh.hpp>
#include <rabbit/platform/window.hpp>

#include <chrono>
#include <filesystem>

using namespace rb;

struct camera_data {
    mat4f projection{ mat4f::perspective(0.6108652382f, 1.7777f, 0.1f, 100.0f) };
    mat4f view{ mat4f::translation({ 0.0f, 0.0f, -5.0f }) };
};

struct local_data {
    mat4f world{ mat4f::identity() };
};

struct material_data {
    vec3f base_color{ 1.0f, 1.0f, 1.0f };
    float roughness{ 0.8f };
    float metallic{ 0.0f };
};

struct test_system : public system {
public:
    test_system(asset_manager& asset_manager, window& window, graphics_device& graphics_device)
        : _asset_manager(asset_manager)
        , _window(window)
        , _graphics_device(graphics_device) {
    }

    void initialize(registry& registry) override {
        _forward_shader = _graphics_device.create_shader(builtin_shaders::get(builtin_shader::forward));

        camera_data camera_data;
        _camera_buffer = _create_uniform_buffer(camera_data);

        local_data local_data;
        _local_buffer = _create_uniform_buffer(local_data);

        material_data material_data;
        _material_buffer = _create_uniform_buffer(material_data);

        _helmet_albedo_map = _asset_manager.load<texture>("textures/helmet/default_albedo_ao.png");

        resource_heap_desc resource_heap_desc;
        resource_heap_desc.shader = _forward_shader;
        resource_heap_desc.resources = {
            { 0, _camera_buffer },
            { 1, _local_buffer },
            { 2, _material_buffer },
            { 3, _helmet_albedo_map }
        };
        _resource_heap = _graphics_device.create_resource_heap(resource_heap_desc);

        _command_buffer = _graphics_device.create_command_buffer();

        _helmet_mesh = _asset_manager.load<mesh>("meshes/helmet.obj");

        _skybox_texture = _asset_manager.load<texture>("cubemaps/daylight.json");
    }

    void update(registry& registry, float elapsed_time) override {
        _rotation += elapsed_time;

        _time += elapsed_time;
        if (_time >= 1.0f) {
            char buffer[128];
            sprintf(buffer, "RabBit FPS: %d", _fps);
            _window.set_title(buffer);

            _fps = 0;
            _time -= 1.0f;
        }
    }

    void draw(registry& registry, graphics_device& graphics_device) override {
        _command_buffer->begin();

        local_data local_data;
        local_data.world = mat4f::rotation_y(_rotation);
        _command_buffer->update_buffer(_local_buffer, &local_data, 0, sizeof(local_data));

        _command_buffer->begin_render_pass(graphics_device);

        _command_buffer->set_shader(_forward_shader);

        _command_buffer->set_resource_heap(_resource_heap);

        _command_buffer->set_vertex_buffer(_helmet_mesh->vertex_buffer());

        _command_buffer->draw(_helmet_mesh->vertex_buffer()->count(), 1, 0, 0);

        _command_buffer->end_render_pass();

        _command_buffer->end();

        graphics_device.submit(_command_buffer);

        _fps++;
    }

private:
    template<typename T>
    std::shared_ptr<buffer> _create_uniform_buffer(const T& data) {
        buffer_desc desc;
        desc.type = buffer_type::uniform;
        desc.size = sizeof(T);
        desc.stride = desc.size;
        desc.data = &data;
        return _graphics_device.create_buffer(desc);
    }

private:
    asset_manager& _asset_manager;
    window& _window;
    graphics_device& _graphics_device;
    std::shared_ptr<shader> _forward_shader;
    std::shared_ptr<resource_heap> _resource_heap;
    std::shared_ptr<buffer> _camera_buffer;
    std::shared_ptr<buffer> _material_buffer;
    std::shared_ptr<buffer> _local_buffer;
    std::shared_ptr<mesh> _helmet_mesh;
    std::shared_ptr<texture> _helmet_albedo_map;
    std::shared_ptr<texture> _skybox_texture;
    std::shared_ptr<command_buffer> _command_buffer;
    int _fps = 0;
    float _time = 0.0f;
    float _rotation = 0.0f;
};

int main(int argc, char* argv[]) {
    std::filesystem::current_path(DATA_DIRECTORY);

    auto app = builder{}
        .singleton<settings>(-1)
        .singleton<window>(window_factory{})
        .singleton<graphics_device>(graphics_device_factory{})
        .singleton<asset_manager>()
        .loader<texture, texture_loader>()
        .loader<mesh, mesh_loader>()
        .system<test_system>()
        .configure([](settings& settings) {
            settings.vsync = true;
        })
        .build();

    return app.run();
}
