#include <rabbit/physics/physics.hpp>
#include <rabbit/core/arena.hpp>

#include <box2d/box2d.h>

#include <memory>

using namespace rb;

struct shape_data {
    std::shared_ptr<b2Shape> shape;
};

struct body_data {
    b2Body* body = nullptr;
};

struct physics::data {
    b2World world = b2World(b2Vec2(0.0f, 100.0f));

    arena<shape_data> shapes;
    arena<body_data> bodies;
};

physics::physics()
    : m_data(new data()) {
}

physics::physics(physics&& physics) noexcept
    : m_data(std::move(physics.m_data)) {
}

physics::~physics() = default;

handle physics::create_circle_shape(float radius) {
    handle id = m_data->shapes.create();

    b2CircleShape shape;
    shape.m_radius = radius;

    m_data->shapes[id].shape = std::make_shared<b2CircleShape>(shape);

    return id;
}

handle physics::create_box_shape(const vec2& extents) {
    handle id = m_data->shapes.create();

    b2PolygonShape shape;
    shape.SetAsBox(extents.x, extents.y);

    m_data->shapes[id].shape = std::make_shared<b2PolygonShape>(shape);

    return id;
}

void physics::destroy_shape(handle id) {
    assert(m_data->shapes.valid(id));

    m_data->shapes.destroy(id);
}

handle physics::create_body(body_type type) {
    handle id = m_data->bodies.create();

    b2BodyDef body_def;

    switch (type) {
        case body_type::type_static: body_def.type = b2_staticBody; break;
        case body_type::type_kinematic: body_def.type = b2_kinematicBody; break;
        case body_type::type_dynamic: body_def.type = b2_dynamicBody; break;
    }

    m_data->bodies[id].body = m_data->world.CreateBody(&body_def);

    return id;
}

void physics::destroy_body(handle id) {
    assert(m_data->bodies.valid(id));

    body_data& data = m_data->bodies[id];

    m_data->world.DestroyBody(data.body);
    data.body = nullptr;

    m_data->bodies.destroy(id);
}

void physics::set_body_shape(handle id, handle shape_id) {
    assert(m_data->bodies.valid(id));
    assert(m_data->shapes.valid(shape_id));

    body_data& body_data = m_data->bodies[id];
    shape_data& shape_data = m_data->shapes[shape_id];

    float density = body_data.body->GetType() == b2_staticBody ? 0.0f : 10.0f;

    body_data.body->CreateFixture(shape_data.shape.get(), density);
}

void physics::set_body_position(handle id, const vec2& position) {
    assert(m_data->bodies.valid(id));

    body_data& body_data = m_data->bodies[id];

    body_data.body->SetTransform({ position.x, position.y }, body_data.body->GetAngle());
}

void physics::set_body_rotation(handle id, float rotation) {
    assert(m_data->bodies.valid(id));

    body_data& body_data = m_data->bodies[id];

    body_data.body->SetTransform(body_data.body->GetPosition(), rotation);
}

vec2 physics::get_body_position(handle id) const {
    assert(m_data->bodies.valid(id));

    body_data& body_data = m_data->bodies[id];

    const b2Vec2& position = body_data.body->GetPosition();
    return { position.x, position.y };
}

float physics::get_body_rotation(handle id) const {
    assert(m_data->bodies.valid(id));

    body_data& body_data = m_data->bodies[id];

    return body_data.body->GetAngle();
}

void physics::simulate(float time_step) {
    m_data->world.Step(time_step, 6, 2);
}
