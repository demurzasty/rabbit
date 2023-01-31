#pragma once 

#include <cmath>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace rb {
    template<class T>
    constexpr T basic_pi = T(3.14159265358979323846);

    template<class T>
    constexpr T basic_tau = basic_pi<T> * T(2);

    template<class T>
    struct basic_vec2 {
        using scalar_type = T;

        static constexpr basic_vec2<T> zero() { return { 0, 0 }; }
        static constexpr basic_vec2<T> one() { return { 1, 1 }; }
        static constexpr basic_vec2<T> axis_x() { return { 1, 0 }; }
        static constexpr basic_vec2<T> axis_y() { return { 0, 1 }; }

        T operator[](std::size_t p_index) const { return (&x)[p_index]; }
        T& operator[](std::size_t p_index) { return (&x)[p_index]; }

        T x, y;
    };

    template<class T>
    struct basic_vec3 {
        using scalar_type = T;

        static constexpr basic_vec3<T> zero() { return { 0, 0, 0 }; }
        static constexpr basic_vec3<T> one() { return { 1, 1, 1 }; }
        static constexpr basic_vec3<T> axis_x() { return { 1, 0, 0 }; }
        static constexpr basic_vec3<T> axis_y() { return { 0, 1, 0 }; }
        static constexpr basic_vec3<T> axis_z() { return { 0, 1, 1 }; }

        T operator[](std::size_t p_index) const { return (&x)[p_index]; }
        T& operator[](std::size_t p_index) { return (&x)[p_index]; }

        basic_vec2<T>& xy() { return *reinterpret_cast<basic_vec2<T>*>(&x); }
        const basic_vec2<T>& xy() const { return *reinterpret_cast<const basic_vec2<T>*>(&x); }

        T x, y, z;
    };

    template<class T>
    struct basic_vec4 {
        using scalar_type = T;

        static constexpr basic_vec4<T> zero() { return { 0, 0, 0, 0 }; }
        static constexpr basic_vec4<T> one() { return { 1, 1, 1, 1 }; }
        static constexpr basic_vec4<T> axis_x() { return { 1, 0, 0, 0 }; }
        static constexpr basic_vec4<T> axis_y() { return { 0, 1, 0, 0 }; }
        static constexpr basic_vec4<T> axis_z() { return { 0, 1, 1, 0 }; }
        static constexpr basic_vec4<T> axis_w() { return { 0, 1, 0, 1 }; }

        T operator[](std::size_t p_index) const { return (&x)[p_index]; }
        T& operator[](std::size_t p_index) { return (&x)[p_index]; }

        basic_vec2<T>& xy() { return *reinterpret_cast<basic_vec2<T>*>(&x); }
        const basic_vec2<T>& xy() const { return *reinterpret_cast<const basic_vec2<T>*>(&x); }

        basic_vec3<T>& xyz() { return *reinterpret_cast<basic_vec3<T>*>(&x); }
        const basic_vec3<T>& xyz() const { return *reinterpret_cast<const basic_vec3<T>*>(&x); }

        T x, y, z, w;
    };

    template<class T>
    struct basic_mat3 {
        using scalar_type = T;

        static constexpr basic_mat3<T> identity() { return { 1, 0, 0, 0, 1, 0, 0, 0, 1 }; }

        T operator[](std::size_t p_index) const { return m[p_index]; }
        T& operator[](std::size_t p_index) { return m[p_index]; }

        T m[9];
    };

    template<class T>
    struct basic_mat4 {
        using scalar_type = T;

        static constexpr basic_mat4<T> identity() { return { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1 }; }

        T operator[](std::size_t p_index) const { return m[p_index]; }
        T& operator[](std::size_t p_index) { return m[p_index]; }

        T m[16];
    };

    template<class T>
    struct basic_mat3x4 {
        using scalar_type = T;

        static constexpr basic_mat3x4<T> identity() { return { 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 }; }

        T operator[](std::size_t p_index) const { return m[p_index]; }
        T& operator[](std::size_t p_index) { return m[p_index]; }

        T m[12];
    };

    template<class T>
    constexpr basic_vec2<T> operator+(const basic_vec2<T>& p_left, const basic_vec2<T>& p_right) {
        return { p_left.x + p_right.x, p_left.y + p_right.y };
    }

    template<class T>
    constexpr basic_vec2<T> operator-(const basic_vec2<T>& p_left, const basic_vec2<T>& p_right) {
        return { p_left.x - p_right.x, p_left.y - p_right.y };
    }

    template<class T>
    constexpr basic_vec2<T> operator*(const basic_vec2<T>& p_left, const basic_vec2<T>& p_right) {
        return { p_left.x * p_right.x, p_left.y * p_right.y };
    }

    template<class T>
    constexpr basic_vec2<T> operator/(const basic_vec2<T>& p_left, const basic_vec2<T>& p_right) {
        return { p_left.x / p_right.x, p_left.y / p_right.y };
    }

    template<class T>
    constexpr basic_vec2<T> operator+(const basic_vec2<T>& p_left, const T& p_scalar) {
        return { p_left.x + p_scalar, p_left.y + p_scalar };
    }

    template<class T>
    constexpr basic_vec2<T> operator-(const basic_vec2<T>& p_left, const T& p_scalar) {
        return { p_left.x - p_scalar, p_left.y - p_scalar };
    }

    template<class T>
    constexpr basic_vec2<T> operator*(const basic_vec2<T>& p_left, const T& p_scalar) {
        return { p_left.x * p_scalar, p_left.y * p_scalar };
    }

    template<class T>
    constexpr basic_vec2<T> operator/(const basic_vec2<T>& p_left, const T& p_scalar) {
        return { p_left.x / p_scalar, p_left.y / p_scalar };
    }

    template<class T>
    constexpr basic_vec3<T> operator+(const basic_vec3<T>& p_left, const basic_vec3<T>& p_right) {
        return { p_left.x + p_right.x, p_left.y + p_right.y, p_left.z + p_right.z };
    }

    template<class T>
    constexpr basic_vec3<T> operator-(const basic_vec3<T>& p_left, const basic_vec3<T>& p_right) {
        return { p_left.x - p_right.x, p_left.y - p_right.y, p_left.z - p_right.z };
    }

    template<class T>
    constexpr basic_vec3<T> operator*(const basic_vec3<T>& p_left, const basic_vec3<T>& p_right) {
        return { p_left.x * p_right.x, p_left.y * p_right.y, p_left.z * p_right.z };
    }

    template<class T>
    constexpr basic_vec3<T> operator/(const basic_vec3<T>& p_left, const basic_vec3<T>& p_right) {
        return { p_left.x / p_right.x, p_left.y / p_right.y, p_left.z / p_right.z };
    }

    template<class T>
    constexpr basic_vec3<T> operator+(const basic_vec3<T>& p_left, const T& p_scalar) {
        return { p_left.x + p_scalar, p_left.y + p_scalar, p_left.z + p_scalar };
    }

    template<class T>
    constexpr basic_vec3<T> operator-(const basic_vec3<T>& p_left, const T& p_scalar) {
        return { p_left.x - p_scalar, p_left.y - p_scalar, p_left.z - p_scalar };
    }

    template<class T>
    constexpr basic_vec3<T> operator*(const basic_vec3<T>& p_left, const T& p_scalar) {
        return { p_left.x * p_scalar, p_left.y * p_scalar, p_left.z * p_scalar };
    }

    template<class T>
    constexpr basic_vec3<T> operator/(const basic_vec3<T>& p_left, const T& p_scalar) {
        return { p_left.x / p_scalar, p_left.y / p_scalar, p_left.z / p_scalar };
    }

    template<class T>
    constexpr basic_vec4<T> operator+(const basic_vec4<T>& p_left, const basic_vec4<T>& p_right) {
        return { p_left.x + p_right.x, p_left.y + p_right.y, p_left.z + p_right.z, p_left.w + p_right.w };
    }

    template<class T>
    constexpr basic_vec4<T> operator-(const basic_vec4<T>& p_left, const basic_vec4<T>& p_right) {
        return { p_left.x - p_right.x, p_left.y - p_right.y, p_left.z - p_right.z, p_left.w - p_right.w };
    }

    template<class T>
    constexpr basic_vec4<T> operator*(const basic_vec4<T>& p_left, const basic_vec4<T>& p_right) {
        return { p_left.x * p_right.x, p_left.y * p_right.y, p_left.z * p_right.z, p_left.w * p_right.w };
    }

    template<class T>
    constexpr basic_vec4<T> operator/(const basic_vec4<T>& p_left, const basic_vec4<T>& p_right) {
        return { p_left.x / p_right.x, p_left.y / p_right.y, p_left.z / p_right.z, p_left.w / p_right.w };
    }

    template<class T>
    constexpr basic_vec4<T> operator+(const basic_vec4<T>& p_left, const T& p_scalar) {
        return { p_left.x + p_scalar, p_left.y + p_scalar, p_left.z + p_scalar, p_left.w + p_scalar };
    }

    template<class T>
    constexpr basic_vec4<T> operator-(const basic_vec4<T>& p_left, const T& p_scalar) {
        return { p_left.x - p_scalar, p_left.y - p_scalar, p_left.z - p_scalar, p_left.w - p_scalar };
    }

    template<class T>
    constexpr basic_vec4<T> operator*(const basic_vec4<T>& p_left, const T& p_scalar) {
        return { p_left.x * p_scalar, p_left.y * p_scalar, p_left.z * p_scalar, p_left.w * p_scalar };
    }

    template<class T>
    constexpr basic_vec4<T> operator/(const basic_vec4<T>& p_left, const T& p_scalar) {
        return { p_left.x / p_scalar, p_left.y / p_scalar, p_left.z / p_scalar, p_left.w / p_scalar };
    }

    template<typename T>
    constexpr basic_mat4<T> operator*(const basic_mat4<T>& p_left, const basic_mat4<T>& p_right) {
        return {
            p_left[0] * p_right[0] + p_left[4] * p_right[1] + p_left[8] * p_right[2] + p_left[12] * p_right[3],
            p_left[1] * p_right[0] + p_left[5] * p_right[1] + p_left[9] * p_right[2] + p_left[13] * p_right[3],
            p_left[2] * p_right[0] + p_left[6] * p_right[1] + p_left[10] * p_right[2] + p_left[14] * p_right[3],
            p_left[3] * p_right[0] + p_left[7] * p_right[1] + p_left[11] * p_right[2] + p_left[15] * p_right[3],

            p_left[0] * p_right[4] + p_left[4] * p_right[5] + p_left[8] * p_right[6] + p_left[12] * p_right[7],
            p_left[1] * p_right[4] + p_left[5] * p_right[5] + p_left[9] * p_right[6] + p_left[13] * p_right[7],
            p_left[2] * p_right[4] + p_left[6] * p_right[5] + p_left[10] * p_right[6] + p_left[14] * p_right[7],
            p_left[3] * p_right[4] + p_left[7] * p_right[5] + p_left[11] * p_right[6] + p_left[15] * p_right[7],

            p_left[0] * p_right[8] + p_left[4] * p_right[9] + p_left[8] * p_right[10] + p_left[12] * p_right[11],
            p_left[1] * p_right[8] + p_left[5] * p_right[9] + p_left[9] * p_right[10] + p_left[13] * p_right[11],
            p_left[2] * p_right[8] + p_left[6] * p_right[9] + p_left[10] * p_right[10] + p_left[14] * p_right[11],
            p_left[3] * p_right[8] + p_left[7] * p_right[9] + p_left[11] * p_right[10] + p_left[15] * p_right[11],

            p_left[0] * p_right[12] + p_left[4] * p_right[13] + p_left[8] * p_right[14] + p_left[12] * p_right[15],
            p_left[1] * p_right[12] + p_left[5] * p_right[13] + p_left[9] * p_right[14] + p_left[13] * p_right[15],
            p_left[2] * p_right[12] + p_left[6] * p_right[13] + p_left[10] * p_right[14] + p_left[14] * p_right[15],
            p_left[3] * p_right[12] + p_left[7] * p_right[13] + p_left[11] * p_right[14] + p_left[15] * p_right[15]
        };
    }

    template<typename T>
    constexpr basic_vec4<T> operator*(const basic_mat4<T>& p_left, const basic_vec4<T>& p_right) {
        return {
            p_right.x* p_left[0] + p_right.y * p_left[4] + p_right.z * p_left[8] + p_right.w * p_left[12],
            p_right.x* p_left[1] + p_right.y * p_left[5] + p_right.z * p_left[9] + p_right.w * p_left[13],
            p_right.x* p_left[2] + p_right.y * p_left[6] + p_right.z * p_left[10] + p_right.w * p_left[14],
            p_right.x* p_left[3] + p_right.y * p_left[7] + p_right.z * p_left[11] + p_right.w * p_left[15]
        };
    }

    template<typename T>
    static constexpr bool is_power_of_to(const T& x) {
        return !(x == 0) && !(x & (x - 1));
    }

    template<typename T>
    static constexpr T next_power_of_two(T v) {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        return ++v;
    }

    template<class T>
    constexpr T min(const T& p_left, const T& p_right) {
        return (p_left < p_right) ? p_left : p_right;
    }

    template<class T>
    constexpr T max(const T& p_left, const T& p_right) {
        return (p_left > p_right) ? p_left : p_right;
    }

    template<class T>
    constexpr bool contains(const basic_vec4<T>& p_rect, const basic_vec2<T>& p_point) {
        return p_point.x >= p_rect.x && p_point.y >= p_rect.y &&
            p_point.x < p_rect.x + p_rect.z &&
            p_point.y < p_rect.y + p_rect.w;
    }

    template<class T>
    constexpr T dot(const basic_vec2<T>& p_left, const basic_vec2<T>& p_right) {
        return p_left.x * p_right.x + p_left.y * p_right.y;
    }

    template<class T>
    constexpr T dot(const basic_vec3<T>& p_left, const basic_vec3<T>& p_right) {
        return p_left.x * p_right.x + p_left.y * p_right.y + p_left.z * p_right.z;
    }

    template<class T>
    constexpr T dot(const basic_vec4<T>& p_left, const basic_vec4<T>& p_right) {
        return p_left.x * p_right.x + p_left.y * p_right.y + p_left.z * p_right.z + p_left.w * p_right.w;
    }

    template<class Vec>
    constexpr auto length_squared(const Vec& p_vec) -> typename Vec::scalar_type {
        return dot(p_vec, p_vec);
    }

    template<class Vec>
    inline auto length(const Vec& p_vec) -> typename Vec::scalar_type {
        return std::sqrt(length_squared(p_vec));
    }

    template<class Vec>
    inline Vec normalize(const Vec& p_vec) {
        return p_vec / length(p_vec);
    }

    template<class Vec>
    inline auto distance(const Vec& p_from, const Vec& p_to) -> typename Vec::scalar_type {
        return length(p_to - p_from);
    }

    template<class Vec>
    inline Vec direction(const Vec& p_from, const Vec& p_to) {
        return normalize(p_to - p_from);
    }

    template<class Vec>
    inline Vec move_towards(const Vec& p_from, const Vec& p_to, const typename Vec::scalar_type& p_delta) {
        return p_from + direction(p_from, p_to) * min(p_delta, distance(p_from, p_to));
    }

    constexpr auto pi = basic_pi<float>;
    constexpr auto tau = basic_tau<float>;

    using vec2 = basic_vec2<float>;
    using vec3 = basic_vec3<float>;
    using vec4 = basic_vec4<float>;

    using ivec2 = basic_vec2<int>;
    using ivec3 = basic_vec3<int>;
    using ivec4 = basic_vec4<int>;

    using uvec2 = basic_vec2<unsigned int>;
    using uvec3 = basic_vec3<unsigned int>;
    using uvec4 = basic_vec4<unsigned int>;

    using mat3 = basic_mat3<float>;
    using mat4 = basic_mat4<float>;
    using mat3x4 = basic_mat3x4<float>;
}
