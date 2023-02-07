#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    window window("demo", { 1280, 720 }, false);
    renderer renderer(window);
    painter painter(renderer, { 320, 180 });
    physics physics;
    registry registry;
    stopwatch stopwatch;

    window.on<close_event>().connect<&window::close>(window);

    texture texture = texture_loader(renderer)("data/characters.png");

    while (window.is_open()) {
        window.dispatch();

        physics.simulate(stopwatch.restart());

        painter.draw(texture, vec2::zero(), color::white());

        renderer.display(color::cornflower_blue());
    }
}
