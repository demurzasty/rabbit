#pragma once

namespace rb {
    /**
     * @brief Defines comparison operators that can be chosen for alpha, stencil, or depth-buffer tests.
     */
    enum class compare_operator {
        /**
         * @brief Always pass the test.
         */
        always,

        /**
         * @brief Always fail the test.
         */
        never,

        /**
         * @brief Accept the new pixel if its value is less than the value of the current pixel.
         */
        less,

        /**
         * @brief Accept the new pixel if its value is less than or equal to the value of the current pixel.
         */
        less_equal,

        /**
         * @brief Accept the new pixel if its value is equal to the value of the current pixel.
         */
        equal,

        /**
         * @brief Accept the new pixel if its value is greater than or equal to the value of the current pixel.
         */
        greater_equal,

        /**
         * @brief Accept the new pixel if its value is greater than the value of the current pixel.
         */
        greater,

        /**
         * @brief Accept the new pixel if its value does not equal the value of the current pixel.
         */
        not_equal
    };
}
