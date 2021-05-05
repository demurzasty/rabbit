#include <rabbit/graphics/shader.hpp>

using namespace rb;

shader::shader(const shader_desc& desc)
    : _vertex_layout(desc.vertex_layout)
    , _bindings(desc.bindings)
    , _render_target(desc.render_target)
    , _topology(desc.topology)
    , _polygon_mode(desc.polygon_mode)
    , _cull_mode(desc.cull_mode)
    , _front_face(desc.front_face)
    , _depth_test_enable(desc.depth_test_enable)
    , _depth_write_enable(desc.depth_write_enable)
    , _stencil_test_enable(desc.stencil_test_enable)
    , _depth_compare_operator(desc.depth_compare_operator) {
}

const vertex_layout& shader::vertex_layout() const {
    return _vertex_layout;
}

const std::vector<shader_binding>& shader::bindings() const {
    return _bindings;
}

const std::shared_ptr<texture>& shader::render_target() const {
    return _render_target;
}

topology shader::topology() const {
    return _topology;
}

polygon_mode shader::polygon_mode() const {
    return _polygon_mode;
}

cull_mode shader::cull_mode() const {
    return _cull_mode;
}

front_face shader::front_face() const {
    return _front_face;
}

bool shader::is_depth_test() const {
    return _depth_test_enable;
}

bool shader::is_depth_write() const {
    return _depth_write_enable;
}

bool shader::is_stencil_test() const {
    return _stencil_test_enable;
}

compare_operator shader::depth_compare_operator() const {
    return _depth_compare_operator;
}