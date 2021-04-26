#include <rabbit/platform/window_factory.hpp>
#include <rabbit/graphics/graphics_device_factory.hpp>
#include <rabbit/core/injector.hpp>
#include <rabbit/engine/settings.hpp>
#include <rabbit/engine/application.hpp>
#include <rabbit/engine/builder.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    auto app = builder{}
        .singleton<settings>()
        .singleton<window>(window_factory{})
        .singleton<graphics_device>(graphics_device_factory{})
        .build();

    return app.run();
}
