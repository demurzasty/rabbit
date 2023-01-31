#pragma once 

#include "window.hpp"
#include "graphics.hpp"

#include <vector>
#include <functional>
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

    struct ui_window {
        vec2 anchor = { 0.0f, 0.0f };
        bool moving = false;
        vec4 rect = { 0.0f, 0.0f, 128.0f, 128.0f };

        std::vector<vertex2d> vertices;
        std::vector<std::uint32_t> indices;
        std::uint32_t index_offset = 0;
    };

    /**
     * @brief An immediate mode graphic user interface.
     */
    class ui {
    public:
        ui(window& p_window, graphics& p_graphics)
            : m_window(p_window), m_graphics(p_graphics) {
            p_window.on_mouse_move().connect<&ui::on_mouse_move>(this);
            p_window.on_mouse_button().connect<&ui::on_mouse_button>(this);
        }

        std::size_t get_id() const {
            return m_ids.empty() ? 0 : m_ids.back();
        }

        std::size_t pop_id() {
            assert(!m_ids.empty());
            const auto id = m_ids.back();
            m_ids.pop_back();
            return id;
        }

        std::size_t push_id(std::size_t p_id) {
            return m_ids.emplace_back(combine_hash(get_id(), p_id));
        }

        std::size_t push_id(std::string_view p_string) {
            std::hash<std::string_view> hash;
            return push_id(hash(p_string));
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

        bool begin_window(std::string_view p_title) {
            const auto id = push_id(p_title);

            auto& window = m_windows[id];

            if (!m_mouse_buttons[mouse_button::left]) {
                window.moving = false;
            }

            if (is_mouse_button_pressed(mouse_button::left) && contains(window.rect, m_mouse_position)) {
                window.anchor = m_mouse_position - vec2{ window.rect.x, window.rect.y };
                window.moving = true;
            }

            if (window.moving && m_mouse_buttons[mouse_button::left]) {
                window.rect.x = m_mouse_position.x - window.anchor.x;
                window.rect.y = m_mouse_position.y - window.anchor.y;
            }

            return color_rect(window.rect, m_colors.frame_background);
        }

        void end_window() {
            pop_id();
        }

        void render() {
            for (auto&& [button, pressed] : m_mouse_buttons) {
                m_last_mouse_buttons[button] = pressed;
            }

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
        std::size_t combine_hash(std::size_t p_seed, std::size_t p_hash) {
            std::hash<std::size_t> hash;
            return p_seed ^ (hash(p_hash) + 0x9e3779b9 + (p_seed << 6) + (p_seed >> 2));
        }

        void on_mouse_move(const mouse_move_event& p_event) {
            m_mouse_position = p_event.position;
        }

        void on_mouse_button(const mouse_button_event& p_event) {
            m_mouse_position = p_event.position;
            m_mouse_buttons[p_event.button] = p_event.pressed;
        }

        bool is_mouse_button_pressed(mouse_button p_button) {
            return m_mouse_buttons[p_button] && !m_last_mouse_buttons[p_button];
        }

    private:
        window& m_window;
        graphics& m_graphics;

        std::vector<std::size_t> m_ids;

        vec2 m_mouse_position = vec2::zero();
        std::unordered_map<mouse_button, bool> m_mouse_buttons;
        std::unordered_map<mouse_button, bool> m_last_mouse_buttons;

        ui_colors m_colors;

        std::unordered_map<std::size_t, ui_window> m_windows;

        std::vector<vertex2d> m_vertices;
        std::vector<std::uint32_t> m_indices;
        std::uint32_t m_index_offset = 0;
    };
}
