#pragma once

namespace rb {
    /**
     * @brief Shader stage flags.
     */
    struct shader_stage_flags {
        enum {
            vertex = (1 << 0),
            fragment = (1 << 1)
        };
    };
}
