#pragma once 

#include "../math/vec2.hpp"

namespace rb {
    /**
     * @brief Window close event.
     *        Emitted when user clicked on close window button.
     */
    struct close_event {
    };

    /**
     * @brief Window dispatch event.
     */
    struct dispatch_event {
    };

    /**
     * @brief Mouse move event.
     */
    struct mouse_move_event {
        /**
         * @brief Mouse position relative to the window.
         */
        vec2 position;
    };

    /**
     * @brief Mouse button enumerator.
     */
    enum class mouse_button : unsigned char {
        /**
         * @brief Left mouse button.
         */
        left,

        /**
         * @brief Right mouse button.
         */
        right,

        /**
         * @brief Middle mouse button.
         */
        middle
    };

    /**
     * @brief Mouse button event.
     */
    struct mouse_button_event {
        /**
         * @brief Mouse position relative to the window.
         */
        vec2 position;

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
