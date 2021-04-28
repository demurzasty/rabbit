#pragma once

#include "camera_type.hpp"

namespace rb {
    struct camera {
        camera_type type{ camera_type::perspective };
        float size{ 11.25 };
        float field_of_view{ 45.0f };
    };
}
