#pragma once

#include "../math/vec2.hpp"

#include <string>

namespace rb {
    /**
     * @brief Application settings.
     */
    struct settings {
        /**
         * @brief Determine initial window size, excluding OS window decorations.
         */
        vec2u window_size{ 1280, 720 };

        /**
         * @brief Determine initial window title.
         */
        std::string window_title{ "RabBit " };
    };
}
