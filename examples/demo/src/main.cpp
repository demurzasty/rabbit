#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    window window("demo", { 1280, 720 }, false);
    renderer renderer(window);
    physics physics;
    registry registry;

    window.on<close_event>().connect<&window::close>(window);

    ref<texture> texture = texture_loader(renderer)("data/buddy.png");

    //ref<sprite> sprite = rb::sprite(renderer, texture);

    for (int i = 0; i < 100; ++i) {
        entity buddy = registry.create();
        registry.emplace<sprite>(buddy, renderer, texture);

        body& body = registry.emplace<rb::body>(buddy, physics, body_type::type_dynamic);
        body.set_position({ 640.0f, 360.0f });
        body.set_shape(box_shape(physics, { 8.0f, 11.0f }));
    }

    stopwatch stopwatch;

    float acc = 0.0f;

    while (window.is_open()) {
        window.dispatch();

        float time_step = stopwatch.restart();
        acc += time_step;

        while (acc >= 1.0f / 60.0f) {
            physics.simulate(1.0f / 60.0f);
            acc -= 1.0f / 60.0f;
        }

        registry.view<body, sprite>().each([](body& body, sprite& sprite) {
            sprite.set_transform(body.transform());
        });

        renderer.display(color::cornflower_blue());
    }
}
