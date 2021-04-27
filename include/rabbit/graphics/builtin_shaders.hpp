#pragma once

#include "fwd.hpp"

namespace rb {
    /**
     * @brief Builtin shaders list.
     */
    enum class builtin_shader {
        forward,
        skybox,
        brdf,
        irradiance,
        prefilter
    };

    /**
     * @brief Builtin shaders structure.
     */
    struct builtin_shaders {
        static shader_desc get(builtin_shader shader);
    };
}
