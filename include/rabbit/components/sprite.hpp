#pragma once 

#include "../graphics/texture.hpp"

namespace rb {
    /**
     * @brief Built-in drawable representation of a texture component.
     */
    struct sprite {
        /**
         * @brief Source texture.
         */
        ref<texture> texture;
    };
}
