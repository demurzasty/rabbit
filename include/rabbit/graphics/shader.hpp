#pragma once

#include "fwd.hpp"
#include "vertex.hpp"
#include "../core/span.hpp"

#include <cstdint>
#include <vector>
#include <optional>

namespace rb {
    struct shader_stage_flags {
        enum {
            vertex = (1 << 0),
            fragment = (1 << 1)
        };
    };

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

    struct shader_desc {
        vertex_desc vertex_desc;
        span<const std::uint32_t> vertex_bytecode;
        span<const std::uint32_t> fragment_bytecode;
        std::vector<shader_binding_desc> bindings;
    };

    class shader {
    public:
        shader(const shader_desc& desc);

        virtual ~shader() = default;

        const vertex_desc& vertex_desc() const;

        std::optional<shader_binding_desc> binding(std::size_t slot) const;

        const std::vector<shader_binding_desc>& bindings() const;

    private:
        rb::vertex_desc _vertex_desc;
        std::vector<shader_binding_desc> _bindings;
    };
}