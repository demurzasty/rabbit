#pragma once

#include "texture_format.hpp"
#include "texture_filter.hpp"
#include "texture_wrap.hpp"
#include "../math/vec2.hpp"

namespace rb {
    struct texture_desc {
        const void* data{ nullptr };
        vec2u size{ 0, 0 };
        texture_format format{ texture_format::rgba8 };
        texture_filter filter{ texture_filter::linear };
        texture_wrap wrap{ texture_wrap::repeat };
    };
}
