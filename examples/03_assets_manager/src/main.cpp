#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    // Create new window.
    window window("hello_world", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer(window);

    // Create painter to dynamically render 2D stuff.
    painter painter(renderer, { 320, 180 });

    // Create assets menager (not required).
    assets assets;
    assets.loader<texture, texture_loader>(renderer);

    // Load texture from file using texture loader.
    ref<texture> texture = assets.load<rb::texture>("data/buddy.png");

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Create stopwatch to use time factor.
    stopwatch stopwatch;

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.dispatch();

        // Draw texture on screen.
        painter.draw(*texture, { stopwatch.time(), 60.0f }, color::white());

        // Render and display it onto a screen.
        renderer.display(color::cornflower_blue());
    }
}
