#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace rb {
    /**
     * @brief Defines vertex element formats.
     */
    enum class vertex_format_type : std::uint8_t {
        integer,
        floating_point
    };

    enum class vertex_attribute : std::uint8_t {
        position,
        texcoord,
        normal,
        blend_weight,
        blend_indices
    };

    struct vertex_format {
        struct hasher {
            std::size_t operator()(const vertex_format& format) const {
                return *reinterpret_cast<const std::uint32_t*>(&format);
            }
        };

        static inline constexpr vertex_format vec2f() {
            return { vertex_format_type::floating_point, 2, 8, false };
        }

        static inline constexpr vertex_format vec3f() {
            return { vertex_format_type::floating_point, 3, 12, false };
        }

        static inline constexpr vertex_format vec4f() {
            return { vertex_format_type::floating_point, 4, 16, false };
        }

        static inline constexpr vertex_format vec2i() {
            return { vertex_format_type::integer, 2, 8, false };
        }

        static inline constexpr vertex_format vec3i() {
            return { vertex_format_type::integer, 3, 12, false };
        }

        static inline constexpr vertex_format vec4i() {
            return { vertex_format_type::integer, 4, 16, false };
        }

        vertex_format_type type{ vertex_format_type::floating_point };
        std::uint8_t components{ 0 };
        std::uint8_t size{ 0 };
        bool normalized{ false };
    };

    struct vertex_element {
        vertex_attribute attribute{ vertex_attribute::position };
        vertex_format format;
    };

    class vertex_layout {
    public:
        using value_type = vertex_element;
        using allocator_type = std::vector<vertex_element>::allocator_type;
        using pointer = std::vector<vertex_element>::pointer;
        using const_pointer = std::vector<vertex_element>::const_pointer;
        using reference = vertex_element&;
        using const_reference = const vertex_element&;
        using size_type = std::vector<vertex_element>::size_type;
        using difference_type = std::vector<vertex_element>::difference_type;
        using difference_type = std::vector<vertex_element>::difference_type;

        using iterator = std::vector<vertex_element>::iterator;
        using const_iterator = std::vector<vertex_element>::const_iterator;

    public:
        vertex_layout() = default;

        vertex_layout(std::initializer_list<vertex_element> elements);

        vertex_layout(const vertex_layout&) = default;
        vertex_layout& operator=(const vertex_layout&) = default;

        vertex_layout(vertex_layout&&) = default;
        vertex_layout& operator=(vertex_layout&&) = default;

        const vertex_element& operator[](std::size_t index) const;

        iterator begin();
        const_iterator begin() const;

        iterator end();
        const_iterator end() const;

        bool empty() const;
        std::size_t size() const;

        std::size_t stride() const;
        std::size_t offset(std::size_t vertex_index) const;

    private:
        std::vector<vertex_element> _elements;
    };
}