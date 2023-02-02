#pragma once 

#include "../graphics/image.hpp"

#include <string_view>

namespace rb {
    /**
     * @brief Basically, you can just use image::from function,
     *        but for convenient we create another utility class
     *        to fit others loaders.
     */
    class image_loader {
    public:
        /**
         * @brief Load new image from file.
         * 
         * @return Loaded image.
         */
        [[nodiscard]] image operator()(std::string_view path) const;
    };
}
