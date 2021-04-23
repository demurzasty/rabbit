#pragma once

#include "../platform/fwd.hpp"

#include <memory>

namespace rb {
    struct graphics_device_desc {
        std::shared_ptr<window> window;
    };

    class graphics_device {
    public:
        graphics_device(const graphics_device_desc& desc);

        virtual ~graphics_device() = default;

        const std::shared_ptr<window>& associated_window() const;

    private:
        std::shared_ptr<window> _window;
    };
}
