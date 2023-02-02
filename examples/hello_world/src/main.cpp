#include <rabbit/rabbit.hpp>

using namespace rb;

struct foo {
    int value = 3;
};

int main(int argc, char* argv[]) {
    // Create new window.
    window window("hello_world", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer{ window };

    // Load texture from file using texture loader.
    texture texture = texture_loader(renderer)("data/characters.png");

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.dispatch_events();

        // Render and display it onto a screen.
        renderer.display(color::cornflower_blue());
    }
}
