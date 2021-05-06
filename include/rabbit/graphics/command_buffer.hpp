
#pragma once

#include "shader.hpp"
#include "resource_heap.hpp"
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
        virtual void begin_render_pass(const std::shared_ptr<texture>& render_target, std::size_t layer) = 0;

        /**
         * @brief Ends the current render pass.
         */
        virtual void end_render_pass() = 0;

        /**
         * @brief Updates the data of the specifed buffer during recording the command buffer.
         *
         * @param buffer Specifes the destination buffer whose data is to be updated.
         * @param data Raw pointer to the data which the buffer is to be updated. This must not be null!
         * @param offset Specifies the destination offset (in bytes) at which the buffer is to be updated.
         * @param size Specifies the sizes (in bytes) of the data block which is to be updated.
         *
         * @warning Record this command outside of a render pass.
         */
        virtual void update_buffer(const std::shared_ptr<buffer>& buffer, const void* data, std::size_t offset, std::size_t size) = 0;

        /**
         * @brief Sets the active viewport.
         *
         * @param viewport Viewport to set.
         */
        virtual void set_viewport(const vec4f& viewport) = 0;

        /**
         * @brief Sets the active shader.
         *
         * @param shader Specifies the shader which is to be bound for subsequent draw or compute commands.
         */
        virtual void set_shader(const std::shared_ptr<shader>& shader) = 0;

        /**
         * @brief Binds the specified resource heap to the active shader.
         *
         * @param resource_heap Specifies the resource heap that contains all shader resources thath will be bound to the shader pipeline.
         *
         * @warning Any previously bound resources are invalid after this call.
         */
        virtual void set_resource_heap(const std::shared_ptr<resource_heap>& resource_heap) = 0;

        /**
         * @brief Sets the specified vertex buffer for subsequent drawing operations.
         *
         * @param vertex_buffer Specifies the vertex buffer to set. This buffer must have been created with the vertex type and its content must not be uninitialized.
         */
        virtual void set_vertex_buffer(const std::shared_ptr<buffer>& vertex_buffer) = 0;

        /**
         * @brief Sets the specified index buffer for subsequent drawing operations.
         *
         * @param index_buffer Specifies the index buffer to set. This buffer must have been created with the index type and its content must not be uninitialized.
         */
        virtual void set_index_buffer(const std::shared_ptr<buffer>& index_buffer) = 0;

        /**
         * @brief Draws the specified amount of instances of primitives from the currently set vertex buffer.
         * @param vertex_count Number of vertices to generate.
         * @param instance_count Number of instances to generate.
         * @param first_vertex Zero-based offset of the first vertex from the vertex buffer.
         * @param first_instance Zero-based offset of the first instance.
         */
        virtual void draw(std::size_t vertex_count, std::size_t instance_count, std::size_t first_vertex, std::size_t first_instance) = 0;

        /**
         * @brief Draws the specified amount of instances of primitives from the currently set vertex buffer.
         * @param index_count Number of vertices to generate.
         * @param instance_count Number of instances to generate.
         * @param first_vertex Zero-based offset of the first vertex from the vertex buffer.
         * @param vertex_offset Zero-based offset of the first vertex from the vertex buffer.
         * @param first_instance Zero-based offset of the first instance.
         */
        virtual void draw_indexed(std::size_t index_count, std::size_t instance_count, std::size_t first_index, std::size_t vertex_offset, std::size_t first_instance) = 0;
    };
}
