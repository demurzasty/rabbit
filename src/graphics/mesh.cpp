#include <rabbit/graphics/mesh.hpp>
#include <rabbit/core/config.hpp>

using namespace rb;

mesh::mesh(const mesh_desc& desc)
    : _topology(desc.topology)
    , _vertex_layout(desc.vertex_layout)
    , _vertex_count(desc.vertex_count)
    , _index_count(desc.index_count)
    , _index_type(desc.index_type) {
    RB_ASSERT(_topology != topology::undefined, "Topology is undefined");
    RB_ASSERT(!_vertex_layout.empty(), "Vertex layout is empty");

    for (RB_MAYBE_UNUSED const auto& vertex_element : _vertex_layout) {
        RB_ASSERT(vertex_element.attribute != vertex_attribute::undefined, "One of vertex attributes is undefined");
        RB_ASSERT(vertex_element.format.type != vertex_type::undefined, "One of vertex format type is undefined");
        RB_ASSERT(vertex_element.format.components > 0, "One of vertex format component count is zero");
        RB_ASSERT(vertex_element.format.size > 0, "One of vertex format size is zero");
    }

    RB_ASSERT(desc.vertex_count > 0, "Vertex count is zero");
    RB_ASSERT(desc.vertex_data, "Vertex data is not provided");
    RB_ASSERT(!desc.index_data || desc.index_count > 0, "Index count is zero");
    RB_ASSERT(!desc.index_data || desc.index_type != index_type::undefined, "Index type is not provided");
}

topology mesh::topology() const {
    return _topology;
}

const vertex_layout& mesh::vertex_layout() const {
    return _vertex_layout;
}

std::size_t mesh::vertex_count() const {
    return _vertex_count;
}

std::size_t mesh::index_count() const {
    return _index_count;
}

index_type mesh::index_type() const {
    return _index_type;
}
