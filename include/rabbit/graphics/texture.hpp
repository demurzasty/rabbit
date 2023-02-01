#pragma once 

#include "renderer.hpp"
#include "../core/handle.hpp"

namespace rb {
    /**
     * @brief A structure that contains an efficient,
     *        driver-specific representation of pixel data.
     *        This is an object-oriented wrapper of texture
     *        created by a renderer.
     */
    class texture {
    public:
        /**
         * @brief Construct a new texture.
         *
         * @param renderer The renderer this texture belongs to.
         */
        texture(renderer& renderer);

        /**
         * @brief Disabled copy constructor.
         */
        texture(const texture&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        texture(texture&&) noexcept = default;

        /**
         * @brief Destructor of the texture.
         */
        virtual ~texture();

        /**
         * @brief Disabled copy assignment.
         */
        texture& operator=(const texture&) = delete;

        /**
         * @brief Enabled move assignment.
         */
        texture& operator=(texture&&) noexcept = default;

        /**
         * @brief Get the size of the texture.
         *
         * @return Size of the texture in pixels.
         */
        [[nodiscard]] uvec2 size() const;

        /**
         * @brief Get the pixel format of the texture.
         *
         * @return Pixel format of the texture.
         */
        [[nodiscard]] pixel_format format() const;

    private:
        renderer& m_renderer;
        handle m_id = null;
    };
}