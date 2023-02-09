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
        body.set_shape(box_shape(physics, { 16.0f, 16.0f }));
    }

    stopwatch stopwatch;
    while (window.is_open()) {
        window.dispatch();

        physics.simulate(stopwatch.restart());

        registry.view<body, sprite>().each([](body& body, sprite& sprite) {
            sprite.set_transform(body.transform());
        });

        // sprite->set_transform(mat2x3::rotation(stopwatch.time()));

        renderer.display(color::cornflower_blue());
    }
}
