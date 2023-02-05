#pragma once 

#include "../graphics/color.hpp"

namespace rb {
    /**
     * @brief Built-in tint component.
     */
    struct tint {
        /**
         * @brief Color tint.
         */
        color color = color::white();
    };
}
