#pragma once

#include "fwd.hpp"
#include "vertex.hpp"
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
        vertex_desc vertex_desc;
        span<const std::uint32_t> vertex_bytecode;
        span<const std::uint32_t> fragment_bytecode;
        std::vector<shader_binding_desc> bindings;
        std::vector<shader_push_constant_desc> push_constants;
    };

    /**
     * @brief Shader interface.
     */
    class shader {
        /**
         * @brief Default virtual destructor.
         */
        virtual ~shader() = default;

        /**
         * @brief Returns vertex layout descriptor.
         */
        const vertex_desc& vertex_desc() const;

        std::optional<shader_binding_desc> binding(std::size_t slot) const;

        const std::vector<shader_binding_desc>& bindings() const;

    protected:
        shader(const shader_desc& desc);

    private:
        const rb::vertex_desc _vertex_desc;
        const std::vector<shader_binding_desc> _bindings;
    };
}