
#pragma once

#include "material.hpp"
#include "mesh.hpp"
#include "../math/vec4.hpp"

#include <memory>

// TODO: Executing secondary command buffer.

namespace rb {
    class graphics_device;

    /**
     * @brief Command buffer interface used for storing and recording GPU commands.
     *
     * This is the main interface to recording graphics, compute, and blit command to be submitted to the GPU.
     * Before any command can be recorded, the command buffer must be set into record mode,
     * which is done by the command_buffer:begin function.
     *
     * @see graphics_device::create_command_buffer
     */
    class command_buffer {
    public:
        /**
         * @brief Default virtual destructor.
         */
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

        /**
         * @brief Begins with a new render pass.
         */
        virtual void begin_render_pass(graphics_device& graphics_device) = 0;

        /**
         * @brief Begins with a new render pass.
         */
        virtual void begin_render_pass(const std::shared_ptr<texture>& render_target) = 0;

        /**
         * @brief Ends the current render pass.
         */
        virtual void end_render_pass() = 0;

        /**
         * @brief Sets the active viewport.
         *
         * @param viewport Viewport to set.
         */
        virtual void set_viewport(const vec4f& viewport) = 0;

        /**
         * @brief Draw the specifed mesh using provided material.
         *
         * @param mesh Mesh to draw.
         * @param material Material to be used.
         */
        virtual void draw(const std::shared_ptr<mesh>& mesh, const std::shared_ptr<material>& material) = 0;
    };
}
