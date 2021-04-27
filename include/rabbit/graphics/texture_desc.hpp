#pragma once

#include "texture_format.hpp"
#include "texture_filter.hpp"
#include "texture_type.hpp"
#include "texture_wrap.hpp"
#include "../math/vec3.hpp"

#include <cstddef>

namespace rb {
    struct texture_desc {
        const void* data{ nullptr };
        vec3u size{ 0, 0, 0 };
        texture_type type{ texture_type::texture_2d };
        texture_format format{ texture_format::rgba8 };
        texture_filter filter{ texture_filter::linear };
        texture_wrap wrap{ texture_wrap::repeat };
        std::size_t mipmaps{ 1 };
        std::size_t layers{ 1 };
        bool is_render_target{ false };
    };
}
