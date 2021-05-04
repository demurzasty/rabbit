
#pragma once

#include "../asset/loader.hpp"
#include "../graphics/graphics_device.hpp"

namespace rb {
    class texture_loader : public loader {
    public:
        texture_loader(graphics_device& graphics_device);

        std::function<std::shared_ptr<void>()> load(const std::string& filename, const json& metadata) override;

    private:
        graphics_device& _graphics_device;
    };
}
