#pragma once

namespace rb {
    /**
     * @brief Defines winding orders that may be used to identify faces as front faces.
     */
    enum class front_face {
        /**
         * @brief Treat counterclockwise faces as front faces.
         */
        counter_clockwise,

        /**
         * @brief Treat clockwise faces as front faces.
         */
        clockwise
    };
}
