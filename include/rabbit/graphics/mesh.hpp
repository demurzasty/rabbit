#pragma once

#include "buffer.hpp"

#include <cstddef>
#include <vector>
#include <memory>

namespace rb {
    enum class vertex_attribute {
        undefined,
        position,
        texcoord,
        normal,
        blend_weight,
        blend_indices
    };

    enum class vertex_type {
        undefined,
        integer,
        floating_point
    };

    struct vertex_format {
        static constexpr vertex_format undefined() {
            return {};
        }

        static constexpr vertex_format vec2f() {
            return { vertex_type::floating_point, 2, 8, false };
        }

        static constexpr vertex_format vec3f() {
            return { vertex_type::floating_point, 3, 12, false };
        }

        static constexpr vertex_format vec4f() {
            return { vertex_type::floating_point, 4, 16, false };
        }

        static constexpr vertex_format vec2i() {
            return { vertex_type::integer, 2, 8, false };
        }

        static constexpr vertex_format vec3i() {
            return { vertex_type::integer, 3, 12, false };
        }

        static constexpr vertex_format vec4i() {
            return { vertex_type::integer, 4, 16, false };
        }

        static constexpr vertex_format color() {
            return { vertex_type::integer, 4, 4, true };
        }

        vertex_type type{ vertex_type::undefined };
        std::size_t components{ 0 };
        std::size_t size{ 0 };
        bool normalized{ false };
    };

    struct vertex_element {
        vertex_attribute attribute{ vertex_attribute::undefined };
        vertex_format format{ vertex_format::undefined() };
    };

    using vertex_layout = std::vector<vertex_element>;

    struct mesh_desc {
        vertex_layout vertex_layout;
        std::shared_ptr<buffer> vertex_buffer;
        std::shared_ptr<buffer> index_buffer;
    };

    class mesh {
    public:
        /**
         * @brief Default virtual destructor.
         */
        virtual ~mesh() = default;

        /**
         * @brief Returns vertex layout.
         */
        const vertex_layout& vertex_layout() const;

        /**
         * @brief Returns vertex buffer.
         */
        const std::shared_ptr<buffer>& vertex_buffer() const;

        /**
         * @brief Returns index buffer.
         */
        const std::shared_ptr<buffer>& index_buffer() const;

    protected:
        mesh(const mesh_desc& desc);

    private:
        const rb::vertex_layout _vertex_layout;
        const std::shared_ptr<buffer> _vertex_buffer;
        const std::shared_ptr<buffer> _index_buffer;
    };
}
