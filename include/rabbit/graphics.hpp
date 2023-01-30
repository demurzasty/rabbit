#pragma once 

#include "window.hpp"

#include <memory>

namespace rb {
    class graphics {
    public:
        graphics(const window& p_window);

        graphics(const graphics&) = delete;

        graphics(graphics&&) noexcept = default;

        ~graphics();

        graphics& operator=(const graphics&) = delete;

        graphics& operator=(graphics&&) noexcept = default;

        void present();

    private:
        struct impl;

    private:
        std::shared_ptr<impl> m_impl;
    };
}
