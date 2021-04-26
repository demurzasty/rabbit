#include <rabbit/graphics/mesh.hpp>

using namespace rb;

mesh::mesh(const mesh_desc& desc)
    : _vertex_layout(desc.vertex_layout)
    , _vertex_buffer(desc.vertex_buffer)
    , _index_buffer(desc.index_buffer) {
}

const rb::vertex_layout& mesh::vertex_layout() const {
    return _vertex_layout;
}

const std::shared_ptr<buffer>& mesh::vertex_buffer() const {
    return _vertex_buffer;
}

const std::shared_ptr<buffer>& mesh::index_buffer() const {
    return _index_buffer;
}
