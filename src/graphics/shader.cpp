#include <rabbit/graphics/shader.hpp>
#include <rabbit/core/config.hpp>

using namespace rb;

shader::shader(const shader_desc& desc)
    : _vertex_desc(desc.vertex_desc)
    , _bindings(desc.bindings) {
    RB_ASSERT(!_vertex_desc.empty(), "Vertex description is not provided");
}

const vertex_desc& shader::vertex_desc() const {
    return _vertex_desc;
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