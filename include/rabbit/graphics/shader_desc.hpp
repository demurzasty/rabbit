#pragma once

#include "fwd.hpp"
#include "vertex_layout.hpp"
#include "topology.hpp"
#include "cull_mode.hpp"
#include "front_face.hpp"
#include "compare_operator.hpp"
#include "polygon_mode.hpp"
#include "../core/span.hpp"
#include "shader_stage_flags.hpp"
#include "shader_binding_desc.hpp"
#include "shader_push_constant_desc.hpp"

#include <vector>

namespace rb {
    /**
     * @brief Shader descriptor structure.
     */
    struct shader_desc {
        vertex_layout vertex_layout;
        span<const std::uint32_t> vertex_bytecode;
        span<const std::uint32_t> fragment_bytecode;
        std::vector<shader_binding_desc> bindings;
        std::vector<shader_push_constant_desc> push_constants;

        struct {
            topology topology{ topology::triangles };
        } input_assembly;

        struct {
            polygon_mode polygon_mode{ polygon_mode::fill };
            cull_mode cull_mode{ cull_mode::back };
            front_face front_face{ front_face::clockwise };
        } rasterizer_state;

        struct {
            bool depth_test_enable{ true };
            bool depth_write_enable{ true };
            bool stencil_test_enable{ false };
            compare_operator compare_operator{ compare_operator::less };
        } depth_stencil_state;
    };
}
