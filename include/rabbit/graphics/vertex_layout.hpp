#pragma once

#include "vertex_element.hpp"

#include <vector>

namespace rb {
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