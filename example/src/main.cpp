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
#include <rabbit/loaders/material_loader.hpp>
#include <rabbit/graphics/graphics_device.hpp>
#include <rabbit/platform/window.hpp>
#include <rabbit/systems/renderer.hpp>
#include <rabbit/engine/entity.hpp>

#include <rabbit/components/camera.hpp>
#include <rabbit/components/geometry.hpp>
#include <rabbit/components/transform.hpp>

#include <rabbit/math/math.hpp>

#include <filesystem>

using namespace rb;

struct test_system : public system {
public:
    test_system(asset_manager& asset_manager, window& window, graphics_device& graphics_device)
        : _asset_manager(asset_manager)
        , _window(window)
        , _graphics_device(graphics_device) {
    }

    void initialize(registry& registry) override {
        auto helmet = registry.create();
        auto& helmet_transform = registry.emplace<transform>(helmet);
        auto& helmet_geometry = registry.emplace<geometry>(helmet);

        helmet_geometry.mesh = _asset_manager.load<mesh>("meshes/helmet.obj");
        helmet_geometry.material = _asset_manager.load<material>("materials/helmet.json");

        auto camera = registry.create();
        registry.emplace<rb::camera>(camera);
        registry.emplace<transform>(camera).position = { 0.0f, 0.0f, 5.0f };
    }

    void update(registry& registry, float elapsed_time) override {
    }

    void draw(registry& registry, graphics_device& graphics_device) override {
    }

private:
    asset_manager& _asset_manager;
    window& _window;
    graphics_device& _graphics_device;

    std::shared_ptr<render_pass> _render_pass;

    std::shared_ptr<shader> _brdf_shader;
    std::shared_ptr<texture> _brdf;
    std::shared_ptr<buffer> _quad_buffer;
    bool _brdf_generated{ false };

    std::shared_ptr<shader> _forward_shader;
    std::shared_ptr<resource_heap> _resource_heap;
    std::shared_ptr<buffer> _camera_buffer;
    std::shared_ptr<buffer> _material_buffer;
    std::shared_ptr<buffer> _local_buffer;
    std::shared_ptr<mesh> _helmet_mesh;
    std::shared_ptr<texture> _helmet_albedo_map;

    std::shared_ptr<texture> _skybox_texture;
    std::shared_ptr<shader> _skybox_shader;
    std::shared_ptr<resource_heap> _skybox_resource_heap;
    std::shared_ptr<buffer> _skybox_vertex_buffer;
    std::shared_ptr<buffer> _skybox_index_buffer;
    std::shared_ptr<buffer> _skybox_camera_buffer;

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
        .loader<material, material_loader>()
        .system<renderer>()
        .configure([](settings& settings) {
            settings.vsync = true;
        })
        .system<test_system>()
        .build();

    return app.run();
}
