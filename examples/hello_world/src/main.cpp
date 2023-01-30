#include <rabbit/rabbit.hpp>

using namespace rb;

void on_close(bool& p_open) {
    p_open = false;
}

int main(int argc, char* argv[]) {
    window window;
    
    bool open = true;
    window.on_close().connect<&on_close>(open);

    while (open) {
        window.dispatch_events();
    }

    return 0;
}
