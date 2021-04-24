#pragma once

#include "fwd.hpp"

#include <memory>

// TODO: Executing secondary command buffer.
// TODO: Begin render pass on render target.

namespace rb {
    class command_buffer {
    public:
        virtual ~command_buffer() = default;

        /**
         * @brief Begins with the recording of this command buffer.
         *
         * All functions of the command buffer interface must be used between begin and end calls.
         */
        virtual void begin() = 0;

        /**
         * @brief Ends the recording of this command buffer.
         *
         * After this call, the command buffer can be submitted to the graphics device.
         */
        virtual void end() = 0;

        virtual void begin_render_pass(const std::shared_ptr<graphics_device>& graphics_device) = 0;

        virtual void end_render_pass() = 0;

        virtual void set_shader(const std::shared_ptr<shader>& shader) = 0;
    };
}
