#include <rabbit/core/builder.hpp>
#include <rabbit/components/camera.hpp>
#include <rabbit/components/geometry.hpp>
#include <rabbit/components/transform.hpp>
#include <rabbit/analitycs/profiler.hpp>

#include <filesystem>

using namespace rb;

class initialize_system : public system {
public:
    initialize_system(asset_manager& asset_manager)
        : _asset_manager(asset_manager) {
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
        registry.view<transform, geometry>().each([elapsed_time](transform& transform, geometry& geometry) {
            transform.rotation.y += elapsed_time;
        });
    }

private:
    asset_manager& _asset_manager;
    profiler _profiler;
};

int main(int argc, char* argv[]) {
    std::filesystem::current_path(DATA_DIRECTORY);

    auto app = builder::create_default({})
        .system<initialize_system>()
        .build();

    return app.run();
}
