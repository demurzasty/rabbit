#include <rabbit/core/builder.hpp>
#include <rabbit/platform/window_provider.hpp>
#include <rabbit/graphics/graphics_device_provider.hpp>
#include <rabbit/asset/asset_manager.hpp>
#include <rabbit/loaders/material_loader.hpp>
#include <rabbit/loaders/mesh_loader.hpp>
#include <rabbit/loaders/texture_loader.hpp>
#include <rabbit/systems/render_system.hpp>

using namespace rb;

builder builder::create_default(const settings& settings) {
    return builder{ settings }
        .singleton<window>(window_provider{})
        .singleton<graphics_device>(graphics_device_provider{})
        .singleton<asset_manager>()
        .loader<material, material_loader>()
        .loader<mesh, mesh_loader>()
        .loader<texture, texture_loader>()
        .system<render_system>();
}

builder::builder(const settings& settings) {
    _installations.emplace(-1, [settings](injector& injector) {
        injector.install<rb::settings>(settings);
    });
}

application builder::build() const {
    return *this;
}
