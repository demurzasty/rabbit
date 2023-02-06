#include <rabbit/physics/shape.hpp>

using namespace rb;

shape::shape(physics& physics, handle id)
    : m_physics(physics), m_id(id) {
}

shape::shape(shape&& shape) noexcept
     : m_physics(shape.m_physics), m_id(shape.m_id) {
    shape.m_id = null;
}

shape::~shape() {
    if (m_id != null) {
        m_physics.destroy_shape(m_id);
    }
}

shape::operator rb::handle() const {
    return m_id;
}

circle_shape::circle_shape(physics& physics, float radius)
    : shape(physics, physics.create_circle_shape(radius)) {
}

circle_shape::circle_shape(circle_shape&& shape) noexcept
    : shape(shape.m_physics, shape.m_id) {
    shape.m_id = null;
}

box_shape::box_shape(physics& physics, const vec2& extents)
    : shape(physics, physics.create_box_shape(extents)) {
}

box_shape::box_shape(box_shape&& shape) noexcept
    : shape(shape.m_physics, shape.m_id) {
    shape.m_id = null;
}
