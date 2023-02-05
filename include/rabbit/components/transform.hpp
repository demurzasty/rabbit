#pragma once 

#include "../math/vec2.hpp"

namespace rb {
    /**
     * @brief Built-in transform component.
     */
    struct transform {
        /**
         * @brief Position in world coordinates.
         */
        vec2 position = vec2::zero();

        /**
         * @brief Rotation in radians.
         */
        float rotation = 0.0f;

        /**
         * @brief Scale.
         */
        vec2 scale = vec2::one();
    };
}
