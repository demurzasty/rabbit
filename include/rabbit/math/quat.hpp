#pragma once

#include "vec3.hpp"

#include <cmath>

namespace rb {
    template<typename T>
    struct quat {
        static constexpr quat<T> identity() noexcept {
            return { 0, 0, 0, 1 };
        }

        static inline quat<T> rotation(const vec3<T>& vec, T angle) {
            const auto rad = angle / 2;
            const auto scale = std::sin(rad);

            return normalize(quat<T>{ vec.x * scale, vec.y * scale, vec.z * scale, std::cos(rad) });
        }

        T x, y, z, w;
    };

    template<typename T>
    constexpr quat<T> conjugate(const quat<T>& quat) {
        return { quat.x, quat.y, quat.z, -quat.w };
    }

    template<typename T>
    constexpr T dot(const quat<T>& a, const quat<T>& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    template<typename T>
    constexpr T length_sqr(const quat<T>& quat) {
        return dot(quat, quat);
    }

    template<typename T>
    inline T length(const quat<T>& quat) {
        return std::sqrt(length_sqr(quat));
    }

    template<typename T>
    inline quat<T> normalize(const quat<T>& quat) {
        const auto len = length(quat);

        if (len == 0) {
            return { 0, 0, 0, 0 };
        }

        return { quat.x / len, quat.y / len, quat.z / len, quat.w / len };
    }

    template<typename T>
    constexpr quat<T> inverse(const quat<T>& quat) {
        return { -quat.x, -quat.y, -quat.z, quat.w };
    }
}
