#pragma once

#include <cstdint>
#include <cstddef>

namespace rb {
    struct shader_push_constant_desc {
        std::uint32_t stage_flags{ 0 };
        std::size_t offset{ 0 };
        std::size_t size{ 0 };
    };
}
