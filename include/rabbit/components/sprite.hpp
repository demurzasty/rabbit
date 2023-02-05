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

        /**
         * @brief Color tint.
         */
        color color = color::white();

        /**
         * @brief Texture's drawing offset.
         */
        vec2 offset = vec2::zero();

        /**
         * @brief Horizontal frames.
         */
        unsigned int hframes = 1;

        /**
         * @brief Vertical frames.
         */
        unsigned int vframes = 1;

        /**
         * @brief Current frame index.
         */
        unsigned int frame = 0;
    };
}
