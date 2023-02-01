#pragma once 

#include <cmath>

namespace rb {
    /**
     * @brief Representation of 2D vectors and points.
     */
    template<typename T>
    struct basic_vec2 {
        /** 
         * @brief X component of the vector.
         */
        T x;

        /**
         * @brief Y component of the vector.
         */
        T y;
    };

    using vec2 = basic_vec2<float>;
    using ivec2 = basic_vec2<int>;
    using uvec2 = basic_vec2<unsigned int>;
}
