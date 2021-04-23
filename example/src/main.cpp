#include <rabbit/platform/window_manager.hpp>
#include <rabbit/platform/window.hpp>

using namespace rb;

int main(int argc, char* argv[]) {
    window_manager window_manager;
    auto window = window_manager.create_window({});

    while (window->is_open()) {
        window->poll_events();
    }
}
