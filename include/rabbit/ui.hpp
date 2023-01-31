#pragma once 

#include "window.hpp"
#include "graphics.hpp"

namespace rb {
    /**
     * @brief An immediate mode graphic user interface.
     */
    class ui {
    public:
        ui(window& p_window, graphics& p_graphics);

        ~ui();

        void begin();

        void end();

        bool begin_window(std::string_view p_title);

        void end_window();

        void render();

    private:
        void on_mouse_move(const mouse_move_event& p_event);

        void on_mouse_button(const mouse_button_event& p_event);

    private:
        window& m_window;
        graphics& m_graphics;
    };
}
