#pragma once 

#include "renderer.hpp"
#include "texture.hpp"
#include "font.hpp"

#include <string_view>

namespace rb {
    /**
     * @brief Painter class to simplify 2D rendering.
     */
    class painter {
    public:
        /**
         * @brief Construct a new painter.
         * 
         * @param renderer Renderer reference used to draw stuff.
         */
        painter(renderer& renderer, const uvec2& viewport_size);

        /**
         * @brief Disable copy constructor.
         */
        painter(const painter&) = delete;

        /**
         * @brief Move constructor.
         */
        painter(painter&&) noexcept = default;

        /**
         * @brief Disable copy assignment.
         */
        painter& operator=(const painter&) = delete;

        /**
         * @brief Move assignment.
         */
        painter& operator=(painter&&) noexcept = delete;

        /**
         * @brief Add draw quad command to the render queue.
         *
         * @param destination A rectangle that specifies (in screen coordinates) the destination for drawing the quad.
         * @param color The color to tint a quad. Use color::white for full color with no tinting.
         */
        void draw(const rect& destination, color color);

        /**
         * @brief Add draw texture quad command to the render queue.
         *
         * @param texture Texture to draw.
         * @param position The position (in screen coordinates) for drawing the quad.
         * @param color The color to tint a quad. Use color::white for full color with no tinting.
         */
        void draw(const texture& texture, const vec2& position, color color);

        /**
         * @brief Add draw texture quad command to the render queue.
         *
         * @param texture Texture to draw.
         * @param source A rectangle that specifies (in pixels) the source pixels from a texture.
         * @param destination A rectangle that specifies (in screen coordinates) the destination for drawing the quad.
         * @param color The color to tint a quad. Use color::white for full color with no tinting.
         */
        void draw(const texture& texture, const irect& source, const rect& destination, color color);

        /**
         * @brief Add draw texture quad command to the render queue.
         *
         * @param texture Texture to draw.
         * @param source A rectangle that specifies (in pixels) the source pixels from a texture.
         * @param destination A rectangle that specifies (in screen coordinates) the destination for drawing the quad.
         * @param color The color to tint a quad. Use color::white for full color with no tinting.
         * @param center Center of destination rectangle.
         * @param rotation Rotation in radians.
         */
        void draw(const texture& texture, const irect& source, const rect& destination, color color, const vec2& center, float rotation);

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
        /** 
         * @brief Renderer.
         */
        renderer& m_renderer;

        /**
         * Viewport size (in screen coordinates).
         */
        uvec2 m_viewport_size = uvec2::zero();
    };
}
