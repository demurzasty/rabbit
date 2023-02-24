#pragma once 

#include "events.hpp"
#include "window.hpp"

#include <unordered_map>

namespace rb {
    /**
     * @brief Input utility class.
     */
    class input {
    public:
        /**
         * @brief Construct a new input manager.
         * 
         * @param window Window to attached.
         */
        input(window& window);

        /** 
         * @brief Destruct the input manager.
         */
        ~input();

        /**
         * @brief Disable copy constructor.
         */
        input(const input&) = delete;

        /**
         * @brief Enable move constructor.
         */
        input(input&&) noexcept = default;

        /**
         * @brief Disable copy assignment.
         */
        input& operator=(const input&) = delete;

        /**
         * @brief Disable move assignment.
         */
        input& operator=(input&&) noexcept = default;

        /** 
         * @brief Tell whether a mouse button is down.
         */
        bool is_mouse_button_down(mouse_button button) const;

        /**
         * @brief Tell whether a mouse button is up.
         */
        bool is_mouse_button_up(mouse_button button) const;

        /**
         * @brief Tell whether a mouse button was pressed
         */
        bool is_mouse_button_pressed(mouse_button button) const;

    private:
        /**
         * @brief On window dispatch callback.
         */
        void on_dispatch();

        /**
         * @brief On mouse move event callback.
         */
        void on_mouse_move(const mouse_move_event& event);

        /**
         * @brief On mouse button event callback.
         */
        void on_mouse_button(const mouse_button_event& event);

        /**
         * @brief Window reference.
         */
        window& m_window;

        /**
         * @brief Keep mouse position.
         */
        vec2 m_mouse_position = vec2::zero();

        /** 
         * @brief Keep last mouse buttons' states.
         */
        mutable std::unordered_map<mouse_button, bool> m_last_mouse_buttons;

        /**
         * @brief Keep current mouse buttons' states.
         */
        mutable std::unordered_map<mouse_button, bool> m_mouse_buttons;
    };
}
