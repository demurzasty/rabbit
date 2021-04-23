#pragma once

#include "fwd.hpp"
#include "../platform/fwd.hpp"
#include "../core/version.hpp"

#include <memory>
#include <string>

namespace rb {
    struct graphics_device_desc {
        std::shared_ptr<window> window;
        std::string application_name{ "RabBit" };
        version application_version{ 1, 0, 0 };
    };

    class graphics_device {
    public:
        graphics_device(const graphics_device_desc& desc);

        virtual ~graphics_device() = default;

        virtual void present() = 0;

        const std::shared_ptr<window>& associated_window() const;

    private:
        std::shared_ptr<window> _window;
    };
}
