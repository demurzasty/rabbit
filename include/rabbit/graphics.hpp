#pragma once 

#include "math.hpp"
#include "config.hpp"
#include "window.hpp"

#include <memory>

namespace rb {
    using color = basic_vec4<unsigned char>;

    struct vertex2d {
        vec2 position;
        vec2 texcoord;
        color color;
    };

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
