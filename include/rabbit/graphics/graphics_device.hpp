#pragma once

#include "fwd.hpp"
#include "../platform/fwd.hpp"
#include "../core/version.hpp"

#include <memory>
#include <string>

namespace rb {
    /**
     * @brief Graphics device descriptor structure.
     */
    struct graphics_device_desc {
        std::shared_ptr<window> window;
        std::string application_name{ "RabBit" };
        version application_version{ 1, 0, 0 };
    };

    /**
     * @brief Performs primitive-based rendering, creates resources, handles system-level variables, and creates shaders.
     *
     * @see graphics_device_manager::create
     */
    class graphics_device {
    public:
        graphics_device(const graphics_device_desc& desc);

        /**
         * @brief Default virtual destructor.
         */
        virtual ~graphics_device() = default;

        virtual std::shared_ptr<command_buffer> create_command_buffer() = 0;

        virtual std::shared_ptr<buffer> create_buffer(const buffer_desc& desc) = 0;

        virtual std::shared_ptr<texture> create_texture(const texture_desc& desc) = 0;

        virtual std::shared_ptr<shader> create_shader(const shader_desc& desc) = 0;

        virtual std::shared_ptr<resource_heap> create_resource_heap(const resource_heap_desc& desc) = 0;

        virtual void submit(const std::shared_ptr<command_buffer>& command_buffer) = 0;

        virtual void present() = 0;

        const std::shared_ptr<window>& associated_window() const;

    private:
        std::shared_ptr<window> _window;
    };
}
