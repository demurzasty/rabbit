#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    window window("demo", { 1280, 720 }, false);
    renderer renderer(window);
    painter painter(renderer, { 320, 180 });
    canvas canvas(painter);
    registry registry;

    window.on<close_event>().connect<&window::close>(window);

    entity hero = registry.create();

    transform& transform = registry.emplace<rb::transform>(hero);
    transform.position = { 160.0f, 90.0f };

    sprite& sprite = registry.emplace<rb::sprite>(hero);
    sprite.texture = texture_loader(renderer)("data/characters.png");
    sprite.hframes = 23;
    sprite.vframes = 4;
    sprite.frame = 23;
    sprite.offset = { 16.0f, 32.0f };

    stopwatch stopwatch;
    while (window.is_open()) {
        window.dispatch();

        canvas.process(registry, stopwatch.restart());

        renderer.display(color::cornflower_blue());
    }
}
