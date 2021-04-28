#pragma once

#include "../math/vec3.hpp"

namespace rb {
    struct transform {
        vec3f position{ 0.0f, 0.0f, 0.0f };
        vec3f rotation{ 0.0f, 0.0f, 0.0f };
        vec3f scaling{ 1.0f, 1.0f, 1.0f };
    };
}
