#pragma once

#include "../core/system.hpp"
#include "../graphics/graphics_device.hpp"
#include "../graphics/shader.hpp"

namespace rb {
    class render_system : public system {
    public:
        render_system(graphics_device& graphics_device);

        void draw(registry& registry) override;

    private:
        std::shared_ptr<shader> _forward_shader;
    };
}
