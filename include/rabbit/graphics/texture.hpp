#pragma once 

#include "renderer.hpp"
#include "../core/handle.hpp"
#include "../core/reference.hpp"

namespace rb {
    /**
     * @brief A structure that contains an efficient,
     *        driver-specific representation of pixel data.
     *        This is an object-oriented wrapper of texture
     *        handle created by a renderer.
     */
    class texture : public reference {
    public:
        /**
         * @brief Disable default construction.
         */
        texture() = delete;

        /**
         * @brief Construct a new texture.
         *
         * @param renderer The renderer this texture belongs to.
         * @param size Texture size in pixels.
         * @param filter Texture filter type.
         * @param format Texture pixel format.
         */
        texture(renderer& renderer, const uvec2& size, texture_filter filter, pixel_format format);

        /**
         * @brief Disabled copy constructor.
         */
        texture(const texture&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        texture(texture&& texture) noexcept;

        /**
         * @brief Destructor of the texture.
         */
        ~texture();

        /**
         * @brief Disabled copy assignment.
         */
        texture& operator=(const texture&) = delete;

        /**
         * @brief Disabled move assignment.
         */
        texture& operator=(texture&&) = delete;

        /**
         * @brief Cast texture directly to texture handle.
         * 
         * @return Texture handle.
         */
        operator handle() const;

        /**
         * @brief Update a texture data.
         *
         * @param pixels Pixels data with pixel layout determined by a format.
         */
        void update(const void* pixels);

        /**
         * @brief Tell whether attached texture handle is valid.
         * 
         * @return True if handle is valid, false otherwise.
         */
        [[nodiscard]] bool valid() const;

        /**
         * @brief Get the handle of the texture.
         *
         * @return Handle of the texture.
         */
        [[nodiscard]] handle id() const;

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
        /**
         * @biref Renderer to which this texture is attached.
         */
        renderer* m_renderer = nullptr;

        /**
         * @brief Texture id created by a renderer.
         */
        handle m_id = null;
    };
}
