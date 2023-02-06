#include <rabbit/rabbit.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    // Create new window.
    window window("ui", { 1280, 720 }, false);

    // Create renderer and attached window to it.
    renderer renderer(window);

    // Create painter to dynamically render 2D stuff.
    painter painter(renderer, { 1280, 720 });

    font font = font_loader(renderer)("data/proggy_clean.ttf");

    ui ui(window, font);

    // Connect window close event to stop main loop.
    window.on<close_event>().connect<&window::close>(window);

    // Create stopwatch to use time factor.
    stopwatch stopwatch;

    // Run our example in loop until close button is pressed.
    while (window.is_open()) {
        // Dispatch window events and run all connected signals.
        window.wait_dispatch();

        ui.begin();

        if (ui.begin_main_menu_bar()) {
            if (ui.begin_menu("File")) {

                ui.end_menu();
            }

            if (ui.begin_menu("Edit")) {

                ui.end_menu();
            }

            if (ui.begin_menu("Tools")) {

                ui.end_menu();
            }

            if (ui.begin_menu("Project")) {

                ui.end_menu();
            }

            if (ui.begin_menu("Window")) {

                ui.end_menu();
            }

            if (ui.begin_menu("Help")) {

                ui.end_menu();
            }

            ui.end_main_menu_bar();
        }

        if (ui.begin_window("Project Explorer")) {

            ui.end_window();
        }

        ui.end();

        ui.draw(renderer);

        // Render and display it onto a screen.
        renderer.display(color::cornflower_blue());
    }
}
