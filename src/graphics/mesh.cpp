#include <rabbit/graphics/mesh.hpp>
#include <rabbit/core/config.hpp>

using namespace rb;

mesh::mesh(const mesh_desc& desc)
    : _vertex_layout(desc.vertex_layout)
    , _vertex_buffer(desc.vertex_buffer)
    , _index_buffer(desc.index_buffer) {
    RB_ASSERT(!_vertex_layout.empty(), "Vertex layout is empty");

    for (RB_MAYBE_UNUSED const auto& vertex_element : _vertex_layout) {
        RB_ASSERT(vertex_element.attribute != vertex_attribute::undefined, "One of vertex attributes is undefined");
        RB_ASSERT(vertex_element.format.type != vertex_type::undefined, "One of vertex format type is undefined");
        RB_ASSERT(vertex_element.format.components > 0, "One of vertex format component count is zero");
        RB_ASSERT(vertex_element.format.size > 0, "One of vertex format size is zero");
    }

    RB_ASSERT(desc.vertex_buffer, "Vertex buffer is not provided");
}

const vertex_layout& mesh::vertex_layout() const {
    return _vertex_layout;
}

const std::shared_ptr<buffer>& mesh::vertex_buffer() const {
    return _vertex_buffer;
}

const std::shared_ptr<buffer>& mesh::index_buffer() const {
    return _index_buffer;
}