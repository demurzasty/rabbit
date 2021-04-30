#pragma once

#include "texture.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "renderer.hpp"

#include <memory>

namespace rb {
    class graphics_device {
    public:
        virtual ~graphics_device() = default;

        /**
         * @brief Creates renderer.
         */
        virtual std::shared_ptr<renderer> create_renderer() = 0;

        /**
         * @brief Creates graphics texture.
         */
        virtual std::shared_ptr<texture> create_texture(const texture_desc& desc) = 0;

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
