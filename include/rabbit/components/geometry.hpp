#pragma once

#include "../asset/handle.hpp"
#include "../graphics/mesh.hpp"
#include "../graphics/material.hpp"

namespace rb {
    struct geometry {
        handle<mesh> mesh;
        handle<material> material;
    };
}
