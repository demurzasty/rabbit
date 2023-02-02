#pragma once 

#include "renderer.hpp"
#include "../core/handle.hpp"

namespace rb {
    /**
     * @brief A structure that contains an efficient,
     *        driver-specific representation of pixel data.
     *        This is an object-oriented wrapper of texture
     *        handle created by a renderer.
     */
    class texture {
    public:
        /**
         * @brief Construct empty texture.
         */
        texture() = default;

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
         * @brief Enabled move assignment.
         */
        texture& operator=(texture&& texture) noexcept = delete;

        /**
         * @brief Cast texture directly to texture handle.
         * 
         * @return Texture handle.
         */
        operator handle() const;

        /**
         * @brief Setup a texture data.
         * 
         * @warning Attempting to change data of invalid texture 
         *          results in undefined behavior.
         *
         * @param size Texture size in pixels.
         * @param format Texture pixel format.
         * @param pixels Pixels data with pixel layout determined by a format.
         */
        void set_data(const uvec2& size, pixel_format format, const void* pixels);

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
         * @warning Attempting to retrieve info from invalid texture 
         *          results in undefined behavior.
         *
         * @return Size of the texture in pixels.
         */
        [[nodiscard]] uvec2 size() const;

        /**
         * @brief Get the pixel format of the texture.
         * 
         * @warning Attempting to retrieve info from invalid texture 
         *          results in undefined behavior.
         *
         * @return Pixel format of the texture.
         */
        [[nodiscard]] pixel_format format() const;

    private:
        renderer* m_renderer = nullptr;
        handle m_id = null;
    };
}
