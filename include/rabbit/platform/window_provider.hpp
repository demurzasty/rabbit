
#pragma once

#include "window.hpp"
#include "../core/injector.hpp"

#include <memory>

namespace rb {
    /**
     * @brief Window provider.
     *
     * @see window
     */
    class window_provider final {
    public:
        std::shared_ptr<window> operator()(injector& injector) const;
    };
}
