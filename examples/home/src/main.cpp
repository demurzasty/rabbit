#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    // Create new window.
    window window("home", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer(window);

    // Create painter to render 2D stuff.
    painter painter(renderer);

    painter.viewport_size = { 640, 360 };

    texture texture = texture_loader(renderer)("data/graphics/player.png");

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Create stopwatch to use time factor.
    stopwatch stopwatch;

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.dispatch_events();

        painter.draw(texture, { 320.0f, 0.0f }, color::white());

        // Render and display it onto a screen.
        renderer.display(color::cornflower_blue());
    }
}
