#pragma once

namespace rb {
    /**
     * @brief Sampling filter enumeration.
     */
    enum class texture_filter {
        /**
         * @brief Take the nearest texture sample.
         */
        nearest,

        /**
         * @brief Interpolate between multiple texture samples.
         */
        linear,

        /**
         * @brief Interpolate between multiple texture samples using anisotropic filtering.
         *
         * @note Fallback to linear filtering when anisotropic filtering is not supported.
         */
        anisotropic
    };
}
