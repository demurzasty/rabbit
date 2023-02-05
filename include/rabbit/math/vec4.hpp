#pragma once 

#include "math.hpp"

namespace rb {
    /**
     * @brief Representation of 4D vectors and points.
     */
    template<typename T>
    struct basic_vec4 {
        /**
         * @brief X component of the vector.
         */
        T x;

        /**
         * @brief Y component of the vector.
         */
        T y;

        /**
         * @brief Z component of the vector.
         */
        T z;

        /**
         * @brief W component of the vector.
         */
        T w;
    };

    using vec4 = basic_vec4<float>;
    using ivec4 = basic_vec4<int>;
    using uvec4 = basic_vec4<unsigned int>;
}
