#pragma once

#include "fwd.hpp"

#include <memory>
#include <vector>

namespace rb {
    class graphics_device_manager final {
    public:
        std::shared_ptr<graphics_device> create(const graphics_device_desc& desc);

    private:
        std::vector<std::shared_ptr<graphics_device>> _graphics_devices;
    };
}
