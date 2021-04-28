#pragma once

#include "fwd.hpp"
#include "../platform/fwd.hpp"
#include "../engine/fwd.hpp"
#include "../core/non_copyable.hpp"

#include <memory>
#include <string>

namespace rb {
    /**
     * @brief Performs primitive-based rendering, creates resources, handles system-level variables, and creates shaders.
     *
     * @see graphics_device_manager::create
     */
    class graphics_device : public non_copyable {
    public:
        graphics_device(settings& settings, window& window);

        /**
         * @brief Default virtual destructor.
         */
        virtual ~graphics_device() = default;

        virtual std::shared_ptr<render_pass> create_render_pass(const render_pass_desc& desc) = 0;

        virtual std::shared_ptr<command_buffer> create_command_buffer() = 0;

        virtual std::shared_ptr<buffer> create_buffer(const buffer_desc& desc) = 0;

        virtual std::shared_ptr<texture> create_texture(const texture_desc& desc) = 0;

        virtual std::shared_ptr<shader> create_shader(const shader_desc& desc) = 0;

        virtual std::shared_ptr<resource_heap> create_resource_heap(const resource_heap_desc& desc) = 0;

        virtual void submit(const std::shared_ptr<command_buffer>& command_buffer) = 0;

        virtual void present() = 0;

        window& associated_window();

    private:
        window& _window;
    };
}
