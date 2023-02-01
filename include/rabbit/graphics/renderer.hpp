#pragma once 

#include "../core/handle.hpp"
#include "../platform/window.hpp"

#include <memory>

namespace rb {
    /**
     * @brief Defines types of pixel formats.
     */
    enum class pixel_format : unsigned char {
        /**
         * @brief Undefined pixel format.
         */
        undefined,

        /**
         * @brief Unsigned 8-bit single channel pixel format.
         */
        r8,

        /**
         * @brief Unsigned 16-bit pixel format using 8 bits per channel.
         */
        rg8,

        /**
         * @brief Unsigned 32-bit pixel format using 8 bits per channel.
         */
        rgba8
    };

    class renderer {
    public:
        /**
         * @brief Opaque platform-specific implementation data structure.
         */
        struct data;

        /**
         * @brief Construct a new renderer.
         *
         * @param window Window which is used for context creation.
         */
        renderer(window& window);

        /**
         * @brief Disabled copy constructor.
         */
        renderer(const renderer&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        renderer(renderer&&) noexcept = default;

        /**
         * @brief Destructor of the renderer.
         */
        virtual ~renderer();

        /**
         * @brief Disabled copy assignment.
         */
        renderer& operator=(const renderer&) = delete;

        /**
         * @brief Enabled move assignment.
         */
        renderer& operator=(renderer&&) noexcept = default;

        /**
         * @brief Create new texture handle. 
         * 
         * @return Handle for newly created texture.
         */
        [[nodiscard]] handle create_texture();

        /**
         * @brief Destroy texture associated with provided handle.
         * 
         * @warning Attempting to destroy a texture that is invalid 
                    or being destroyed results in undefined behavior.
         *
         * @param id Texture handle.
         */
        void destroy_texture(handle id);

        /**
         * @brief Setup a texture created before.
         * 
         * @warning Attempting to setup a texture that is invalid 
         *          or being destroyed results in undefined behavior.
         *          Providing pixels data with no corresponding pixel layout
         *          results in undefined behavior.
         * 
         * @param id Texture handle.
         * @param size Texture size in pixels.
         * @param format Texture pixel format.
         * @param pixels Pixels data with pixel layout determined by a format.
         */
        void set_texture_data(handle id, const uvec2& size, pixel_format format, const void* pixels);

        /**
         * @brief Get texture size.
         * 
         * @warning Attempting to fetch a texture that is invalid
         *          or being destroyed results in undefined behavior.
         * 
         * @remarks For newly created texture this function alwayrs return zero size.
         * 
         * @return Texture size in pixels.
         */
        [[nodiscard]] uvec2 get_texture_size(handle id) const;

        /**
         * @brief Get texture pixel format.
         *
         * @warning Attempting to fetch a texture that is invalid
         *          or being destroyed results in undefined behavior.
         *
         * @remarks For newly created texture this function alwayrs return pixel_format::undefined.
         *
         * @return Texture pixel format.
         */
        [[nodiscard]] pixel_format get_texture_format(handle id) const;

        /**
         * @brief Render and display result onto a window surface.
         */
        void display();

    private:
        /**
         * @brief Platform-specific implementation data of the window.
         */
        std::unique_ptr<data> m_data;

        /**
         * @brief Keep window reference.
         */
        window& m_window;
    };
}
