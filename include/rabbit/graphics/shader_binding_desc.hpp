#pragma once

#include "shader_binding_type.hpp"

#include <cstdint>
#include <cstddef>

namespace rb {
    /**
     * @brief Directly shader binding description used in shader.
     */
    struct shader_binding_desc {
        shader_binding_type binding_type{ shader_binding_type::uniform_buffer };
        std::uint32_t stage_flags{ 0 };
        std::size_t slot{ 0 };
        std::size_t array_size{ 1 };
        // std::string name; // ? Need in legacy API (GLES2)
    };
}
