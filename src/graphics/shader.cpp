#include <rabbit/graphics/shader.hpp>
#include <rabbit/core/config.hpp>

using namespace rb;

shader::shader(const shader_desc& desc)
    : _vertex_layout(desc.vertex_layout)
    , _bindings(desc.bindings) {
    RB_ASSERT(!_vertex_layout.empty(), "Vertex description is not provided");
}

const vertex_layout& shader::vertex_layout() const {
    return _vertex_layout;
}

std::optional<shader_binding_desc> shader::binding(std::size_t slot) const {
    for (auto& binding : _bindings) {
        if (binding.slot == slot) {
            return binding;
        }
    }

    return {};
}

const std::vector<shader_binding_desc>& shader::bindings() const {
    return _bindings;
}