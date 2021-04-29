#include <rabbit/graphics/resource_heap.hpp>

using namespace rb;

resource_heap::resource_heap(const resource_heap_desc& desc)
    : _shader(desc.shader)
    , _resources(desc.resources) {
}

const std::shared_ptr<shader>& resource_heap::associated_shader() const {
    return _shader;
}
