#pragma once

#include "buffer.hpp"
#include "vertex_layout.hpp"

#include <memory>

namespace rb {
    struct mesh_desc {
        vertex_layout vertex_layout;
        std::shared_ptr<buffer> vertex_buffer;
        std::shared_ptr<buffer> index_buffer;
    };
}
