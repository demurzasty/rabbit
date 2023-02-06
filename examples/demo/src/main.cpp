#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    window window("demo", { 1280, 720 }, false);
    renderer renderer(window);
    physics physics;
    registry registry;
    stopwatch stopwatch;

    window.on<close_event>().connect<&window::close>(window);

    while (window.is_open()) {
        window.dispatch();

        physics.simulate(stopwatch.restart());

        renderer.display(color::cornflower_blue());
    }
}
