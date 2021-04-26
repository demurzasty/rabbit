#pragma once

#include "fwd.hpp"
#include "../core/fwd.hpp"

#include <memory>
#include <vector>

namespace rb {
    /**
     * @brief Chooses a graphics device implementation depending on the using platform.
     */
    class graphics_device_factory final {
    public:
        std::shared_ptr<graphics_device> operator()(injector& injector) const;
    };
}
