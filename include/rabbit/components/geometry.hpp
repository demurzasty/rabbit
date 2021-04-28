#pragma once

#include <rabbit/graphics/fwd.hpp>

#include <memory>

namespace rb {
    struct geometry {
        std::shared_ptr<mesh> mesh;
        std::shared_ptr<material> material;
    };
}
