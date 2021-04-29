#include <rabbit/graphics/command_buffer.hpp>

using namespace rb;

void command_buffer::set_shader(const std::shared_ptr<shader>& shader) {
    _shader = shader;
}

void command_buffer::set_resource_heap(const std::shared_ptr<resource_heap>& resource_heap) {
    _resource_heap = resource_heap;
}

void command_buffer::set_vertex_buffer(const std::shared_ptr<buffer>& vertex_buffer) {
    _vertex_buffer = vertex_buffer;
}

void command_buffer::set_index_buffer(const std::shared_ptr<buffer>& index_buffer) {
    _index_buffer = index_buffer;
}
