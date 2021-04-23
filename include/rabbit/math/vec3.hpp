#pragma once

#include "fwd.hpp"

namespace rb {
    template<typename T>
    struct vec3 {
        static constexpr vec3<T> zero() { return { 0, 0, 0 } };
        static constexpr vec3<T> one() { return { 1, 1, 1 } };
        static constexpr vec3<T> up() { return { 0, 1, 0 } };
        static constexpr vec3<T> down() { return { 0, -1, 0 } };
        static constexpr vec3<T> left() { return { -1, 0, 0 } };
        static constexpr vec3<T> right() { return { 1, 0, 0 } };
        static constexpr vec3<T> forward() { return { 0, 0, 1 } };
        static constexpr vec3<T> back() { return { 0, 0, -1 } };

        T x, y, z;
    };

    /**
     * @brief Calculates the dot product of two vectors.
     *
     * If the two vectors are unit vectors, the dot product returns a floating point value between
     * -1 and 1 that can be used to determine some properties of the angle between two vectors.
     * For example, it can show whether the vectors are orthogonal, parallel,
     * or have an acute or obtuse angle between them.
     *
     * @param a Source vector.
     * @param b Source vector.
     *
     * @warning Because of floating point error,
     *          two orthogonal vectors may not return a dot product that is exactly zero.
     *
     * @return The dot product of the two vectors.
     */
    template<typename T>
    constexpr T dot(const vec3<T>& a, const vec3<T>& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
}
