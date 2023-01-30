#pragma once 

#include "reactive.hpp"

#include <memory>

namespace rb {
    class window {
    public:
        struct close_event {};

    public:
        window();

        window(const window&) = delete;

        window(window&&) noexcept = default;

        ~window();

        window& operator=(const window&) = delete;

        window& operator=(window&&) noexcept = default;

        bool dispatch_events();

        void* handle() const;

        auto on_close() {
            return m_dispatcher.sink<close_event>();
        }

    private:
        struct impl;

    private:
        std::shared_ptr<impl> m_impl;
        dispatcher m_dispatcher;
    };
}
