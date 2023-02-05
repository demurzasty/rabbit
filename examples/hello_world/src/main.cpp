#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    // Create new window.
    window window("hello_world", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer(window);

    // Create canvas to render 2D stuff.
    canvas canvas(renderer);

    // Create assets menager (not required).
    assets assets;
    assets.loader<font, font_loader>(renderer);
    assets.loader<texture, texture_loader>(renderer);

    // Load font from file using font loader.
    ref<font> font = assets.load<rb::font>("data/proggy_clean.ttf");

    // Load texture from file using texture loader.
    ref<texture> texture = assets.load<rb::texture>("data/characters.png");

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Create stopwatch to use time factor.
    stopwatch stopwatch;

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.dispatch_events();

        // Draw texture on screen.
        canvas.draw(*texture, { 0, 32, 32, 32 }, { stopwatch.time() * 32.0f, 296.0f, 128.0f, 128.0f }, color::white(), { 64.0f, 64.0f }, stopwatch.time());

        // Draw text on screen.
        canvas.draw(*font, 39, "Hello World", { 16.0f, 32.0f }, color::white());

        // Render and display it onto a screen.
        renderer.display(color::cornflower_blue());
    }
}
