#pragma once

#include "buffer.hpp"
#include "command_buffer.hpp"
#include "texture.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "../core/non_copyable.hpp"

#include <memory>

namespace rb {
    class graphics_device : public non_copyable {
    public:
        virtual ~graphics_device() = default;

        /**
         * @brief Creates renderer.
         */
        virtual std::shared_ptr<renderer> create_renderer() = 0;

        /**
         * @brief Creates command buffer.
         */
        virtual std::shared_ptr<command_buffer> create_command_buffer() = 0;

        /**
         * @brief Creates buffer.
         */
        virtual std::shared_ptr<buffer> create_buffer(const buffer_desc& desc) = 0;

        /**
         * @brief Creates graphics texture.
         */
        virtual std::shared_ptr<texture> create_texture(const texture_desc& desc) = 0;

        /**
         * @brief Creates shader.
         */
        virtual std::shared_ptr<shader> create_shader(const shader_desc& desc) = 0;

        /**
         * @brief Creates mesh.
         */
        virtual std::shared_ptr<mesh> create_mesh(const mesh_desc& desc) = 0;

        /**
         * @brief Creates material.
         */
        virtual std::shared_ptr<material> create_material(const material_desc& desc) = 0;

        /**
         * @brief Swap buffers.
         */
        virtual void present() = 0;
    };
}
