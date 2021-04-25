#pragma once

#include "fwd.hpp"
#include "vertex_layout.hpp"
#include "topology.hpp"
#include "cull_mode.hpp"
#include "front_face.hpp"
#include "compare_operator.hpp"
#include "polygon_mode.hpp"
#include "../core/span.hpp"

#include <cstdint>
#include <vector>
#include <optional>

namespace rb {
    /**
     * @brief Shader stage flags.
     */
    struct shader_stage_flags {
        enum {
            vertex = (1 << 0),
            fragment = (1 << 1)
        };
    };

    /**
     * @brief Shader binding type.
     */
    enum class shader_binding_type {
        uniform_buffer,
        texture
    };

    /**
     * @brief Directly shader binding description used in shader.
     */
    struct shader_binding_desc {
        shader_binding_type binding_type{ shader_binding_type::uniform_buffer };
        std::uint32_t stage_flags{ 0 };
        std::size_t slot{ 0 };
        std::size_t array_size{ 1 };
        // std::string name; // ? Need in legacy API (GLES2)
    };

    struct shader_push_constant_desc {
        std::uint32_t stage_flags{ 0 };
        std::uint32_t offset{ 0 };
        std::uint32_t size{ 0 };
        // std::string name; // ? Need in legacy API (GLES2)
    };

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

    /**
     * @brief Shader interface.
     */
    class shader {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~shader() = default;

        /**
         * @brief Returns vertex layout descriptor.
         */
        const vertex_layout& vertex_layout() const;

        std::optional<shader_binding_desc> binding(std::size_t slot) const;

        const std::vector<shader_binding_desc>& bindings() const;

    protected:
        shader(const shader_desc& desc);

    private:
        const rb::vertex_layout _vertex_layout;
        const std::vector<shader_binding_desc> _bindings;
    };
}