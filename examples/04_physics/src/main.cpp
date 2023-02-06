#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    // Create new window.
    window window("physics", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer(window);

    // Create painter to dynamically render 2D stuff.
    painter painter(renderer, { 320, 180 });

    // Create physics module.
    physics physics;

    // Load texture from file using texture loader.
    texture buddy = texture_loader(renderer)("data/buddy.png");

    texture mockup = texture_loader(renderer)("data/mockup.png");

    // Create box shape.
    ref<box_shape> shape = box_shape(physics, { 8.0f, 11.0f });

    // Create physics body.
    std::vector<body> bodies;

    for (int i = 0; i < 128; ++i) {
        body body(physics, body_type::type_dynamic);
        body.set_shape(shape);
        body.set_position({ 160.0f + std::sin(i * 43.0f) * 48.0f, 90.0f - i * 32.0f});
        body.set_rotation(i * 62.0f);
        bodies.push_back(std::move(body));
    }

    body static_body(physics, body_type::type_static);
    static_body.set_shape(box_shape(physics, { 160.0f, 16.0f }));
    static_body.set_position({ 160.0f, 144.0f });

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Create stopwatch to use time factor.
    stopwatch stopwatch;

    float accumulation = 0.0f;

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.dispatch();

        // Calculate delta time between frames.
        float time_step = stopwatch.restart();

        accumulation += time_step;

        while (accumulation > 1.0f / 60.0f) {
            // Simulate the world.
            physics.simulate(1.0f / 60.0f);

            accumulation -= 1.0f / 60.0f;
        }

        painter.draw(mockup, vec2::zero(), color::white());

        for (handle body : bodies) {
            vec2 position = physics.get_body_position(body);
            float rotation = physics.get_body_rotation(body);

            // Draw texture on screen.
            painter.draw(buddy, { 0, 0, 32, 32 }, { position.x - 16.0f, position.y - 21.0f, 32.0f, 32.0f }, color::white(), { 16.0f, 21.0f }, rotation);
        }

        // Render and display it onto a screen.
        renderer.display(color::cornflower_blue());
    }
}
