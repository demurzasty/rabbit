#pragma once 

#include "color.hpp"
#include "../math/vec2.hpp"

namespace rb {
    /**
     * @brief Default 2D vertex structure.
     */
    struct vertex2d {
        /**
         * @brief Position of the vertex.
         */
        vec2 position;

        /**
         * @brief Texture coordinates (UV) of the vertex.
         */
        vec2 texcoord;

        /**
         * @brief Color of the vertex.
         */
        color color;
    };
}
