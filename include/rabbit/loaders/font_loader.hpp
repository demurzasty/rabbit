#pragma once 

#include "../graphics/renderer.hpp"
#include "../graphics/font.hpp"

#include <string_view>

namespace rb {
    /**
     * @brief Utility class for loading fonts from files.
     */
    class font_loader {
    public:
        /**
         * @brief Construct new font loader.
         *
         * @param renderer Reference to renderer.
         */
        font_loader(renderer& renderer);

        /**
         * @brief Disabled copy constructor.
         */
        font_loader(const font_loader&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        font_loader(font_loader&&) noexcept = default;

        /**
         * @brief Disabled copy assignment.
         */
        font_loader& operator=(const font_loader&) = delete;

        /**
         * @brief Enabled move assignment.
         */
        font_loader& operator=(font_loader&&) noexcept = default;

        /**
         * @brief Load new font from file.
         *
         * @return Loaded font.
         */
        [[nodiscard]] font operator()(std::string_view path) const;

    private:
        /**
         * @brief Renderer reference.
         */
        renderer& m_renderer;
    };
}
