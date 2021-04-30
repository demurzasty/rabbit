#pragma once

namespace rb {
    template<typename T>
    struct vec4 {
        static constexpr vec4<T> zero() { return { 0, 0, 0, 0 } };
        static constexpr vec4<T> one() { return { 1, 1, 1, 1 } };

        T x, y, z, w;
    };

    using vec4i = vec4<int>;
    using vec4u = vec4<unsigned int>;
    using vec4f = vec4<float>;
}
