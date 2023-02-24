#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    // Create new window.
    window window("input", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer(window);

    // Create input utility class to avoid using signals.
    input input(window);

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.dispatch();

        // Pick a color depends on mouse button state.
        color clear_color = input.is_mouse_button_pressed(mouse_button::left) ?
            color::cornflower_blue() :
            color::black();

        // Render and display it onto a screen.
        renderer.display(clear_color);
    }
}
