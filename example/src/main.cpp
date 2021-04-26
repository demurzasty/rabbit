#include <rabbit/platform/window_factory.hpp>
#include <rabbit/graphics/graphics_device_factory.hpp>
#include <rabbit/core/injector.hpp>
#include <rabbit/engine/settings.hpp>
#include <rabbit/engine/application.hpp>
#include <rabbit/engine/builder.hpp>
#include <rabbit/engine/system.hpp>
#include <rabbit/asset/asset_manager.hpp>
#include <rabbit/loaders/texture_loader.hpp>

using namespace rb;

struct test_system : public system {
public:
    void initialize(registry& registry) override {
        printf("hello\n");
    }
};

int main(int argc, char* argv[]) {
    auto app = builder{}
        .singleton<settings>()
        .singleton<window>(window_factory{})
        .singleton<graphics_device>(graphics_device_factory{})
        .singleton<asset_manager>()
        .loader<texture, texture_loader>()
        .system<test_system>()
        .build();

    return app.run();
}
