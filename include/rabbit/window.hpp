#pragma once 
 
#include "math.hpp"
#include "reactive.hpp"

#include <string>
#include <memory>

namespace rb {
    struct close_event {};

    struct mouse_move_event {
        vec2 position;
    };

    enum class mouse_button {
        left,
        middle,
        right
    };

    struct mouse_button_event {
        vec2 position;
        mouse_button button;
        bool pressed;
    };

    class window {
    public:
        window(const std::string& p_title = "RabBit", int p_width = 1280, int p_height = 720, bool p_fullscreen = false);

        window(const window&) = delete;

        window(window&&) noexcept = default;

        ~window();

        window& operator=(const window&) = delete;

        window& operator=(window&&) noexcept = default;

        void dispatch_events();

        auto on_close() {
            return m_dispatcher.sink<close_event>();
        }

        auto on_mouse_move() {
            return m_dispatcher.sink<mouse_move_event>();
        }

        auto on_mouse_button() {
            return m_dispatcher.sink<mouse_button_event>();
        }

        void* handle() const;

        int width() const;

        int height() const;

    private:
        struct impl;

    private:
        std::shared_ptr<impl> m_impl;
        dispatcher m_dispatcher;
    };
}
