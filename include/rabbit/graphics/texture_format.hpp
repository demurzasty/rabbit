#pragma once

namespace rb {
    /**
     * @brief Defines various types of texture formats.
     */
    enum class texture_format {
        undefined,

        /**
         * @brief 8-bit R pixel format with one channel
         */
        r8,

        /**
         * @brief 16-bit RG pixel format, using 8 bits per channel.
         */
        rg8,

        /**
         * @brief 32-bit RGBA pixel format with alpha, using 8 bits per channel.
         */
        rgba8,

        /**
         * @brief 32-bit depth-stencil pixel format, using 24 bits for depth, and 8 bits for stencil.
         */
        d24s8
    };
}
