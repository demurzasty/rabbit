#include <rabbit/graphics/vertex.hpp>
#include <rabbit/core/config.hpp>

#include <numeric>

using namespace rb;

vertex_desc::vertex_desc(std::initializer_list<vertex_element> elements)
    : _elements(elements) {
    _elements.shrink_to_fit();
}

const vertex_element& vertex_desc::operator[](std::size_t index) const {
    RB_ASSERT(index < size(), "Out of bound");
    return _elements[index];
}

vertex_desc::iterator vertex_desc::begin() {
    return _elements.begin();
}

vertex_desc::const_iterator vertex_desc::begin() const {
    return _elements.begin();
}

vertex_desc::iterator vertex_desc::end() {
    return _elements.end();
}

vertex_desc::const_iterator vertex_desc::end() const {
    return _elements.end();
}

bool vertex_desc::empty() const {
    return _elements.empty();
}

std::size_t vertex_desc::size() const {
    return _elements.size();
}

std::size_t vertex_desc::stride() const {
    const auto element_sum = [](std::size_t sum, const vertex_element& element) {
        return sum + element.format.size;
    };

    return std::accumulate(_elements.begin(), _elements.end(), std::size_t{ 0 }, element_sum);
}

std::size_t vertex_desc::offset(std::size_t vertex_index) const {
    RB_ASSERT(vertex_index < size(), "Out of bound");

    const auto element_sum = [](std::size_t sum, const vertex_element& element) {
        return sum + element.format.size;
    };

    return std::accumulate(_elements.begin(), _elements.begin() + vertex_index, std::size_t{ 0 }, element_sum);
}
