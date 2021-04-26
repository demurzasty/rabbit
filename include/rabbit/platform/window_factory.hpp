#pragma once

#include "fwd.hpp"
#include "../core/fwd.hpp"

#include <memory>
#include <vector>

namespace rb {
    /**
     * @brief Window manager. Use it to create and manage windows.
     *
     * @see window
     */
    class window_factory final {
    public:
        std::shared_ptr<window> operator()(injector& injector) const;
    };
}
