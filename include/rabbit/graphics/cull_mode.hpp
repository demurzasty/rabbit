#pragma once

namespace rb {
    /**
     * @brief Defines which faces should be culled.
     */
    enum class cull_mode {
        /**
         * @brief Do not cull faces.
         */
        none,

        /**
         * @brief Cull front faces.
         */
        front,

        /**
         * @brief Cull back faces.
         */
        back,

        /**
         * @brief Cull front and back faces.
         */
        front_and_back
    };
}
