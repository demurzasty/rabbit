
#pragma once

#include "../asset/loader.hpp"
#include "../asset/asset_manager.hpp"
#include "../graphics/graphics_device.hpp"

namespace rb {
    class material_loader : public loader {
    public:
        material_loader(asset_manager& asset_manager, graphics_device& graphics_device);

        std::shared_ptr<void> load(const std::string& filename, const json& metadata) override;

    private:
        asset_manager& _asset_manager;
        graphics_device& _graphics_device;
    };
}
