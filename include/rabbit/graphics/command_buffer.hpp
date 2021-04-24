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

        virtual void update_buffer(const std::shared_ptr<buffer>& buffer, const void* data, std::size_t offset, std::size_t size) = 0;

        virtual void end_render_pass() = 0;

        virtual void set_shader(const std::shared_ptr<shader>& shader) = 0;

        virtual void set_resource_heap(const std::shared_ptr<resource_heap>& resource_heap) = 0;

        virtual void set_vertex_buffer(const std::shared_ptr<buffer>& vertex_buffer) = 0;

        virtual void draw(std::size_t vertex_count, std::size_t instance_count, std::size_t first_vertex, std::size_t first_instance) = 0;
    };
}
