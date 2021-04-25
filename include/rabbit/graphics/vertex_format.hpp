#pragma once

#include "vertex_format_type.hpp"

#include <cstddef>

namespace rb {
    struct vertex_format {
        static inline constexpr vertex_format vec2f() {
            return { vertex_format_type::floating_point, 2, 8, false };
        }

        static inline constexpr vertex_format vec3f() {
            return { vertex_format_type::floating_point, 3, 12, false };
        }

        static inline constexpr vertex_format vec4f() {
            return { vertex_format_type::floating_point, 4, 16, false };
        }

        static inline constexpr vertex_format vec2i() {
            return { vertex_format_type::integer, 2, 8, false };
        }

        static inline constexpr vertex_format vec3i() {
            return { vertex_format_type::integer, 3, 12, false };
        }

        static inline constexpr vertex_format vec4i() {
            return { vertex_format_type::integer, 4, 16, false };
        }

        vertex_format_type type{ vertex_format_type::floating_point };
        std::size_t components{ 0 };
        std::size_t size{ 0 };
        bool normalized{ false };
    };
}
