#pragma once 

#include "graphics.hpp"

#include <string_view>

namespace rb {
    /**
     * @brief An immediate mode graphic user interface.
     */
    class ui {
    public:
        bool rect(float p_x, float p_y, float p_width, float p_height, color p_color = { 255, 255, 255, 255 }) {
            m_vertices.push_back({ { p_x, p_y }, vec2::zero(), p_color });
            m_vertices.push_back({ { p_x, p_y + p_height }, vec2::zero(), p_color });
            m_vertices.push_back({ { p_x + p_width, p_y + p_height }, vec2::zero(), p_color });
            m_vertices.push_back({ { p_x + p_width, p_y }, vec2::zero(), p_color });
            
            m_indices.push_back(m_index_offset);
            m_indices.push_back(m_index_offset + 1);
            m_indices.push_back(m_index_offset + 2);
            m_indices.push_back(m_index_offset + 2);
            m_indices.push_back(m_index_offset + 3);
            m_indices.push_back(m_index_offset);

            m_index_offset += 6;
            return true;
        }

        bool button(const std::string_view& p_label) {
            return rect(0.0f, 0.0f, p_label.size() * 8.0f, 16.0f);
        }

        void render(graphics& p_graphics) {
            p_graphics.push_canvas_primitives(vertices(), indices());

            m_vertices.clear();
            m_indices.clear();
            m_index_offset = 0;
        }

        span<const vertex2d> vertices() const {
            return m_vertices;
        }

        span<const std::uint32_t> indices() const {
            return m_indices;
        }

    private:
        std::vector<vertex2d> m_vertices;
        std::vector<std::uint32_t> m_indices;
        std::uint32_t m_index_offset = 0;
    };
}
