#pragma once

#include "fwd.hpp"

#include <memory>
#include <vector>

namespace rb {
    /**
     * @brief Window manager. Use it to create and manage windows.
     *
     * @see window
     */
    class window_manager {
    public:
        std::shared_ptr<window> create_window(const window_desc& desc);

    private:
        std::vector<std::shared_ptr<window>> _windows;
    };
}
