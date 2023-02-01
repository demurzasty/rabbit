#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    window window("hello_world", { 1280, 720 }, false);
    renderer renderer{ window };

    window.on<close_event>().connect<&window::close>(window);

    while (window.is_open()) {
        window.dispatch_events();

        renderer.display(color::cornflower_blue());
    }
}
