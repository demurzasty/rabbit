#include <rabbit/platform/input.hpp>

using namespace rb;

input::input(window& window)
    : m_window(window) {
    m_window.on<dispatch_event>().connect<&input::on_dispatch>(this);
    m_window.on<mouse_move_event>().connect<&input::on_mouse_move>(this);
    m_window.on<mouse_button_event>().connect<&input::on_mouse_button>(this);
}

input::~input() {
    m_window.on<mouse_button_event>().disconnect<&input::on_mouse_button>(this);
    m_window.on<mouse_move_event>().disconnect<&input::on_mouse_move>(this);
    m_window.on<dispatch_event>().disconnect<&input::on_dispatch>(this);
}

bool input::is_mouse_button_down(mouse_button button) const {
    return m_mouse_buttons[button];
}

bool input::is_mouse_button_up(mouse_button button) const {
    return !m_mouse_buttons[button];
}

bool input::is_mouse_button_pressed(mouse_button button) const {
    return m_mouse_buttons[button] && !m_last_mouse_buttons[button];
}

void input::on_dispatch() {
    for (auto&& [button, pressed] : m_mouse_buttons) {
        m_last_mouse_buttons[button] = pressed;
    }
}

void input::on_mouse_move(const mouse_move_event& event) {
    m_mouse_position = event.position;
}

void input::on_mouse_button(const mouse_button_event& event) {
    m_mouse_buttons[event.button] = event.pressed;
}
