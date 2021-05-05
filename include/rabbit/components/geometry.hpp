#pragma once

#include "../graphics/mesh.hpp"
#include "../graphics/material.hpp"

#include <memory>

namespace rb {
    struct geometry {
        std::shared_ptr<mesh> mesh;
        std::shared_ptr<material> material;
    };
}
