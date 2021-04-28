#pragma once

#include "../asset/fwd.hpp"
#include "../asset/loader.hpp"
#include "../graphics/fwd.hpp"

namespace rb {
    class material_loader : public loader {
    public:
        material_loader(asset_manager& asset_manager);

        std::shared_ptr<void> load(const std::string& filename, const json& metadata) override;

    private:
        asset_manager& _asset_manager;
    };
}
