#pragma once 

#include "vec2.hpp"

namespace rb {
    /**
     * @brief Representation of 4D vectors and points.
     */
    template<typename T>
    struct basic_vec4 {
        /**
         * @brief Get position (xy).
         * 
         * @return X, Y components.
         */
        const basic_vec2<T>& position() const { return *(const basic_vec2<T>*)(&x); }

        /**
         * @brief Get size (xy).
         *
         * @return Z, W components.
         */
        const basic_vec2<T>& size() const { return *(const basic_vec2<T>*)(&z); }

        /** 
         * @brief Check if point is inside a rectangle.
         * 
         * @return True if rectangle contains point, false otherwise.
         */
        bool contains(const basic_vec2<T>& point) const {
            return point.x >= x && point.x < x + z &&
                point.y >= y && point.y < y + w;
        }

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
