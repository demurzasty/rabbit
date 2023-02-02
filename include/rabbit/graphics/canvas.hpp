#pragma once 

#include "renderer.hpp"
#include "texture.hpp"
#include "font.hpp"

#include <string_view>

namespace rb {
    /**
     * @brief Canvas class to simplify 2D rendering.
     */
    class canvas {
    public:
        /**
         * @brief Construct a new canvas.
         * 
         * @param renderer Renderer reference used to draw stuff.
         */
        canvas(renderer& renderer);

        /**
         * @brief Disable copy constructor.
         */
        canvas(const canvas&) = delete;

        /**
         * @brief Move constructor.
         */
        canvas(canvas&&) noexcept = default;

        /**
         * @brief Disable copy assignment.
         */
        canvas& operator=(const canvas&) = delete;

        /**
         * @brief Move assignment.
         */
        canvas& operator=(canvas&&) noexcept = delete;

        /**
         * @brief Add draw quad command to the render queue.
         *
         * @param destination A rectangle that specifies (in screen coordinates) the destination for drawing the quad.
         * @param color The color to tint a quad. Use color::white for full color with no tinting.
         */
        void draw(const vec4& destination, color color);

        /**
         * @brief Add draw quad command to the render queue.
         *
         * @param texture Texture to draw.
         * @param source A rectangle that specifies (in pixels) the source pixels from a texture.
         * @param destination A rectangle that specifies (in screen coordinates) the destination for drawing the quad.
         * @param color The color to tint a quad. Use color::white for full color with no tinting.
         */
        void draw(const texture& texture, const ivec4& source, const vec4& destination, color color);

        /**
         * @brief Add draw text command to the render queue.
         *
         * @param font Font used to draw text.
         * @param size Font size.
         * @param text Text to draw.
         * @param position A position that specifies (in screen coordinates) the destination for drawing the text.
         * @param color The color to tint a quad. Use color::white for full color with no tinting.
         */
        void draw(const font& font, unsigned char size, std::string_view text, const vec2& position, color color);

    private:
        renderer& m_renderer;
    };
}
