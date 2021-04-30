#include <rabbit/core/application.hpp>
#include <rabbit/core/builder.hpp>
#include <rabbit/platform/window.hpp>
#include <rabbit/graphics/graphics_device.hpp>

#include <chrono>

using namespace rb;

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
    auto& graphics = _injector.get<rb::graphics_device>();

    for (auto& system : _systems) {
        system->initialize(_registry);
    }
    auto last_time = std::chrono::steady_clock::now();
    auto accumulation_time = 0.0f;

    while (window.is_open()) {
        window.poll_events();

        const auto current_time = std::chrono::steady_clock::now();
        const auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(current_time - last_time).count();
        last_time = current_time;

        for (auto& system : _systems) {
            system->update(_registry, elapsed_time);
        }

        accumulation_time += elapsed_time;

        while (accumulation_time >= settings.fixed_time) {
            for (auto& system : _systems) {
                system->fixed_update(_registry, settings.fixed_time);
            }

            accumulation_time -= settings.fixed_time;
        }

        for (const auto& system : _systems) {
            system->draw(_registry);
        }

        graphics.present();
    }

    return 0;
}
