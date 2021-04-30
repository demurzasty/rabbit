#include <rabbit/systems/render_system.hpp>

using namespace rb;

render_system::render_system(graphics_device& graphics_device)
    : _renderer(graphics_device.create_renderer()) {
}

void render_system::draw(registry& registry) {
    _renderer->render(registry);
}
