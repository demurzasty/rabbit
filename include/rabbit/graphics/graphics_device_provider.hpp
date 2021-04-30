#pragma once

#include "graphics_device.hpp"
#include "../core/injector.hpp"

#include <memory>
#include <vector>

namespace rb {
    /**
     * @brief Chooses a graphics device implementation depending on the using platform.
     */
    class graphics_device_provider final {
    public:
        std::shared_ptr<graphics_device> operator()(injector& injector) const;
    };
}
