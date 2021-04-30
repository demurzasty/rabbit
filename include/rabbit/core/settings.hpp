
#pragma once

#include "../math/vec2.hpp"

#include <string>

namespace rb {
    /**
     * @brief Application settings.
     */
    struct settings {
        /**
         * @brief Application name.
         */
        std::string application_name{ "RabBit" };

        /**
         * @brief Determine initial window size, excluding OS window decorations.
         */
        vec2u window_size{ 1280, 720 };

        /**
         * @brief Determine initial window title.
         */
        std::string window_title{ "RabBit " };

        /**
         * @brief Determine that vertical synchronization should be enabled.
         */
        bool vsync{ true };

        /**
         * @brief Determine fixed time interval.
         */
        float fixed_time{ 1.0f / 60.0f };
    };
}
