#pragma once

namespace rb {
    /**
     * @brief Technique for resolving texture coordinates that are outside of the range [0, 1].
     */
    enum class texture_wrap {
        /**
         * @brief Clamp texture coordinates to the interval [0, 1].
         */
        clamp,

        /**
         * @brief Repeat texture coordinates within the interval [0, 1)
         */
        repeat
    };
}
