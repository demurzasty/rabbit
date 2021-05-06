#pragma once

#include "../graphics/texture.hpp"

namespace rb {
    enum class camera_type {
        orthogonal,
        perspective
    };

    struct camera {
        camera_type type{ camera_type::perspective };
        float size{ 11.25 };
        float field_of_view{ 45.0f };
        std::shared_ptr<texture> skybox;
    };
}
