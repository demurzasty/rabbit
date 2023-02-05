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

    /**
     * @brief Rotate a point around another point.
     * 
     * @param pivot The point around which we will rotate another point.
     * @param point Point to rotate around pivot.
     * @param angle Rotation angle in radians.
     * 
     * @return Rotated point around a pivot.
     */
    template<typename T>
    basic_vec2<T> rotate(const basic_vec2<T>& pivot, const basic_vec2<T>& point, T angle) {
        // Compute sinus and cosinus from angle in radians.
        T s = std::sin(angle);
        T c = std::cos(angle);

        // Translate point back to origin.
        basic_vec2<T> origin{ point.x - pivot.x, point.y - pivot.y };

        // Rotate point around a pivot.
        T x = origin.x * c - origin.y * s;
        T y = origin.x * s + origin.y * c;

        // Translate point back.
        return { x + pivot.x, y + pivot.y };
    }

    using vec2 = basic_vec2<float>;
    using ivec2 = basic_vec2<int>;
    using uvec2 = basic_vec2<unsigned int>;
}
