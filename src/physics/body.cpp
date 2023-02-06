#include <rabbit/physics/body.hpp>

using namespace rb;

body::body(physics& physics, body_type type)
    : m_physics(physics), m_id(physics.create_body(type)) {
}

body::body(body&& body) noexcept
    : m_physics(body.m_physics), m_id(body.m_id), m_shape(std::move(body.m_shape)) {
    body.m_id = null;
}

body::~body() {
    if (m_id != null) {
        m_physics.destroy_body(m_id);
    }
}

body::operator rb::handle() const {
    return m_id;
}

void body::set_shape(ref<shape> shape) {
    m_shape = shape;
    m_physics.set_body_shape(m_id, shape ? handle(*shape) : null);
}

void body::set_position(const vec2& position) const {
    m_physics.set_body_position(m_id, position);
}

void body::set_rotation(float rotation) const {
    m_physics.set_body_rotation(m_id, rotation);
}

vec2 body::position() const {
    return m_physics.get_body_position(m_id);
}

float body::rotation() const {
    return m_physics.get_body_rotation(m_id);
}
