#pragma once 

#include "entity.hpp"
#include "graphics.hpp"

namespace rb {
    struct transform {
        vec3 position = vec3::zero();
        vec3 rotation = vec3::zero();
        vec3 scaling = vec3::one();
    };

    struct sprite {
        texture texture = null;
    };
}
