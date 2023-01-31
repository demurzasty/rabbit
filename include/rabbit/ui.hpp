#pragma once 

#include "window.hpp"
#include "graphics.hpp"

#include <string_view>
#include <unordered_map>

namespace rb {
    struct ui_colors {
        color text = { 240, 239, 241, 255 };
        color frame_background = { 60, 63, 65, 255 };
        color frame_border = { 51, 51, 51, 255 };
        color frame_highlight = { 81, 81, 81, 255 };
        color button_default = { 60, 63, 65, 255 };
        color button_hover = { 122, 128, 132, 255 };
        color button_active = { 92, 92, 92, 255 };
    };

    /**
     * @brief An immediate mode graphic user interface.
     */
    class ui {
    public:
        ui(window& p_window, graphics& p_graphics)
            : m_window(p_window), m_graphics(p_graphics) {
            p_window.on_mouse_move().connect<&ui::on_mouse_move>(this);
        }

        bool color_rect(const vec4& p_rect, color p_color = { 255, 255, 255, 255 }) {
            m_vertices.push_back({ { p_rect.x, p_rect.y }, vec2::zero(), p_color });
            m_vertices.push_back({ { p_rect.x, p_rect.y + p_rect.w }, vec2::zero(), p_color });
            m_vertices.push_back({ { p_rect.x + p_rect.z, p_rect.y + p_rect.w }, vec2::zero(), p_color });
            m_vertices.push_back({ { p_rect.x + p_rect.z, p_rect.y }, vec2::zero(), p_color });
            
            m_indices.push_back(m_index_offset);
            m_indices.push_back(m_index_offset + 1);
            m_indices.push_back(m_index_offset + 2);
            m_indices.push_back(m_index_offset + 2);
            m_indices.push_back(m_index_offset + 3);
            m_indices.push_back(m_index_offset);

            m_index_offset += 4;
            return true;
        }

        bool button(const std::string_view& p_label) {
            const vec4 rect = { 0.0f, 0.0f, p_label.size() * 8.0f, 16.0f };
            const color color = contains(rect, m_mouse_position) ? m_colors.button_hover : m_colors.button_default;
            return color_rect(rect, color);
        }

        bool main_menu_bar() {
            color_rect({ 0.0f, 0.0f, float(m_window.width()), 24.0f }, m_colors.frame_background);

            color_rect({ 0.0f, 23.0f, float(m_window.width()), 1.0f }, m_colors.frame_border);

            return true;
        }

        bool dockspace(const std::string_view& p_name) {
            return true;
        }

        void render() {
            m_graphics.push_canvas_primitives(vertices(), indices());

            m_vertices.clear();
            m_indices.clear();
            m_index_offset = 0;
        }

        ui_colors& colors() {
            return m_colors;
        }

        const ui_colors& colors() const {
            return m_colors;
        }

        span<const vertex2d> vertices() const {
            return m_vertices;
        }

        span<const std::uint32_t> indices() const {
            return m_indices;
        }

    private:
        void on_mouse_move(const mouse_move_event& p_event) {
            m_mouse_position = p_event.position;
        }

    private:
        window& m_window;
        graphics& m_graphics;

        vec2 m_mouse_position = vec2::zero();

        ui_colors m_colors;

        std::vector<vertex2d> m_vertices;
        std::vector<std::uint32_t> m_indices;
        std::uint32_t m_index_offset = 0;
    };
}
