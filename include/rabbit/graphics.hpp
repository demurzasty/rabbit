#pragma once 

#include "math.hpp"
#include "config.hpp"
#include "window.hpp"
#include "span.hpp"
#include "memory.hpp"

namespace rb {
    using color = basic_vec4<unsigned char>;

    namespace colors {
        constexpr color white = { 255, 255, 255, 255 };
        constexpr color black = { 0, 0, 0, 255 };
        constexpr color red = { 255, 0, 0, 255 };
        constexpr color green = { 0, 255, 0, 255 };
        constexpr color blue = { 0, 0, 255, 255 };
        constexpr color cornflower_blue = { 100, 149, 237, 255 };
    }

    struct vertex2d {
        vec2 position;
        vec2 texcoord;
        color color;
    };

    enum class texture : id_type {};

    class graphics : public non_copyable {
    public:
        graphics(const window& p_window);

        graphics(graphics&&) noexcept = default;

        ~graphics();

        graphics& operator=(graphics&&) noexcept = default;

        texture create_texture();

        void destroy_texture(texture p_id);

        void set_texture_data(texture p_id, int p_width, int p_height, const void* p_pixels);

        int get_texture_width(texture p_id) const;

        int get_texture_height(texture p_id) const;

        void push_canvas_clip(float p_left, float p_top, float p_width, float p_height);

        void push_canvas_primitives(texture p_texture_id, const span<const vertex2d>& p_vertices, const span<const std::uint32_t>& p_indices);

        void present();

    private:
        struct impl;

    private:
        std::shared_ptr<impl> m_impl;
    };
}
