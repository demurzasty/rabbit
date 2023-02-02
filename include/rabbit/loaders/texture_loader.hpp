#pragma once 

#include "../graphics/renderer.hpp"
#include "../graphics/texture.hpp"

#include <string_view>

namespace rb {
    /**
     * @brief Utility class for loading textures from files.
     */
    class texture_loader {
    public:
        /**
         * @brief Construct new texture loader.
         * 
         * @param renderer Reference to renderer.
         */
        texture_loader(renderer& renderer);

        /**
         * @brief Disabled copy constructor.
         */
        texture_loader(const texture_loader&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        texture_loader(texture_loader&&) noexcept = default;

        /**
         * @brief Disabled copy assignment.
         */
        texture_loader& operator=(const texture_loader&) = delete;

        /**
         * @brief Enabled move assignment.
         */
        texture_loader& operator=(texture_loader&&) noexcept = default;

        /**
         * @brief Load new texture from file.
         *
         * @return Loaded texture.
         */
        [[nodiscard]] texture operator()(std::string_view path) const;

    private:
        /**
         * @brief Renderer reference.
         */
        renderer& m_renderer;
    };
}
