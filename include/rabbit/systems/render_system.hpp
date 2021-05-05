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
        void _create_forward_shader();

    private:
        graphics_device& _graphics_device;
        std::shared_ptr<shader> _forward_shader;
        std::shared_ptr<command_buffer> _command_buffer;
    };
}
