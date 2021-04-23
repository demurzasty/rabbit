#pragma once

namespace rb {
    template<typename T>
    struct vec2;

    using vec2i = vec2<int>;
    using vec2u = vec2<unsigned int>;
    using vec2f = vec2<float>;

    template<typename T>
    struct vec3;

    using vec3i = vec3<int>;
    using vec3u = vec3<unsigned int>;
    using vec3f = vec3<float>;

    template<typename T>
    struct vec4;

    using vec4i = vec4<int>;
    using vec4u = vec4<unsigned int>;
    using vec4f = vec4<float>;

    template<typename T>
    struct mat3;

    using mat3f = mat3<float>;

    template<typename T>
    struct mat4;

    using mat4f = mat4<float>;
}
