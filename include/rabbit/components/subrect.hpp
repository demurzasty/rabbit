#pragma once 

#include "../math/vec4.hpp"

namespace rb {
    /**
     * @brief Built-in subrect component.
     */
    struct subrect {
        /**
         * @brief Sub-rectangle of the texture.
         */
        ivec4 rect;
    };
}
