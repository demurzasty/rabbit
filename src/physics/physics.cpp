#include <rabbit/physics/physics.hpp>

#include <box2d/box2d.h>

using namespace rb;

struct physics::data {
    b2World world = b2World(b2Vec2(0.0f, -10.0f));
};

physics::physics()
    : m_data(new data()) {
}

physics::~physics() {
}
