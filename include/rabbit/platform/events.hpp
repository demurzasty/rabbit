#pragma once 

#include "input.hpp"
#include "../math/vec2.hpp"

namespace rb {
    /**
     * @brief Window close event.
     */
    struct close_event {
    };

    /**
     * @brief Mouse move event.
     */
    struct mouse_move_event {
        /**
         * @brief Mouse position relative to the window.
         */
        ivec2 position;
    };

    /**
     * @brief Mouse button event.
     */
    struct mouse_button_event {
        /**
         * @brief Mouse position relative to the window.
         */
        ivec2 position;

        /**
         * @brief Which mouse button has been pressed or released.
         */
        mouse_button button;

        /** 
         * @brief Tell whether button has been pressed or released.
         */
        bool pressed;
    };
}
