#pragma once 

#include "platform.hpp"
#include "graphics.hpp"

#include <vector>
#include <string_view>
#include <unordered_map>

namespace rb {
    enum class ui_dockpanel_axis {
        horizontal,
        vertical
    };

    struct ui_dockpanel {
        ui_dockpanel_axis axis;
    };

    /**
     * @brief An immediate mode graphic user interface.
     */
    class ui {
    public:
        ui(window& p_window, graphics& p_graphics)
            : m_window(p_window), m_graphics(p_graphics) {
        }

        std::size_t get_id() {
            return !m_ids.empty() ? m_ids.back() : 0;
        }

        std::size_t push_id(std::size_t p_id) {
            return m_ids.emplace_back(combine_hash(get_id(), p_id));
        }

        std::size_t push_id(std::string_view p_string) {
            std::hash<std::string_view> hash;
            return push_id(hash(p_string));
        }

        std::size_t pop_id() {
            assert(!m_ids.empty());

            const auto id = m_ids.back();
            m_ids.pop_back();
            return id;
        }

        bool begin_dockpanel(std::string_view p_title, ui_dockpanel_axis p_axis) {
            const auto id = push_id(p_title);
            auto& dockpanel = m_dockpanels[id];
            dockpanel.axis = p_axis;
            return true;
        }

        void end_dockpanel() {
            pop_id();
        }

        void render() {

        }

    private:
        std::size_t combine_hash(std::size_t p_seed, std::size_t p_id) {
            std::hash<std::size_t> hash;
            return p_seed ^ (hash(p_id) + 0x9e3779b9 + (p_seed << 6) + (p_seed >> 2));
        }

        void on_mouse_move(const mouse_move_event& p_event) {

        }

        void on_mouse_button(const mouse_button_event& p_event) {

        }

    private:
        window& m_window;
        graphics& m_graphics;

        std::vector<std::size_t> m_ids;
        std::unordered_map<std::size_t, ui_dockpanel> m_dockpanels;
    };
}
