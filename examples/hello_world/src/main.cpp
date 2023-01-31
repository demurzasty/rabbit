#include <rabbit/rabbit.hpp>

using namespace rb;

void on_close(bool& p_open) {
    p_open = false;
}

int main(int argc, char* argv[]) {
    window window;
    graphics graphics{ window };
    ui ui{ window, graphics };

    bool open = true;
    window.on_close().connect<&on_close>(open);

    while (open) {
        window.dispatch_events();

        if (ui.begin_window("Inspector")) {

            ui.end_window();
        }

        ui.render();

        graphics.present();
    }

    return 0;
}
