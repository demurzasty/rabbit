#pragma once

#include "fwd.hpp"

namespace rb {
    enum class builtin_shader {
        forward
    };

    struct builtin_shaders {
        static shader_desc get(builtin_shader shader);
    };
}
