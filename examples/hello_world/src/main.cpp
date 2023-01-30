#include <rabbit/rabbit.hpp>

using namespace rb;

void on_close(bool& p_open) {
    p_open = false;
}

int main(int argc, char* argv[]) {
    window window;
    graphics graphics{ window };
    
    bool open = true;
    window.on_close().connect<&on_close>(open);

    arena<int> arena;

    arena.create(5);

    arena.destroy(arena.create(3));

    while (open) {
        window.dispatch_events();

        graphics.present();
    }

    return 0;
}
