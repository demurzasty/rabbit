#pragma once 

#include "reactive.hpp"

#include <string>
#include <memory>

namespace rb {
    class window {
    public:
        struct close_event {};

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
