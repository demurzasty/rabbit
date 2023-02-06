#pragma once 

#include "../core/stopwatch.hpp"
#include "../graphics/renderer.hpp"
#include "../graphics/font.hpp"

#include <stack>
#include <vector>
#include <string_view>
#include <unordered_map>

namespace rb {
    /** 
     * @brief Immediate mode user interface.
     */
    class ui {
    public:
        /**
         * @brief Construct a new ui.
         */
        ui(window& window, font& font);

        /**
         * @brief Destruct the ui.
         */
        ~ui();

        /**
         * @brief Get ID from top of the stack.
         * 
         * @return ID from top of the stack or 0 if stack is empty.
         */
        std::size_t get_id() const;

        /**
         * @brief Pop ID from stack and return top ID.
         * 
         * @warning Popping empty stack can results in undefined behavior.
         * 
         * @return Top ID.
         */
        std::size_t pop_id();

        /**
         * @brief Push new ID onto a stack and return new combined ID.
         * 
         * @note Onto a stack will be pushed combined ID with ID on top of the stack.
         * 
         * @param id ID to push. 
         * 
         * @return Combined ID.
         */
        std::size_t push_id(std::size_t id);

        /**
         * @brief Push new ID computed from a text onto a stack and return new combined ID.
         *
         * @note Onto a stack will be pushed combined ID with ID on top of the stack.
         *
         * @param id ID to push.
         *
         * @return Combined ID.
         */
        std::size_t push_id(std::string_view text);

        /**
         * @brief Begin.
         */
        void begin();

        /**
         * @brief End.
         */
        void end();

        /**
         * @brief Begin main menu bar.
         */
        bool begin_main_menu_bar();

        /**
         * @brief End main menu bar.
         */
        void end_main_menu_bar();

        /**
         * @brief Begin new menu.
         * 
         * @param label Menu label.
         */
        bool begin_menu(std::string_view label);

        /**
         * @brief End last menu.
         */
        void end_menu();

        /**
         * @brief Begin window.
         * 
         * @param title Title of the window.
         */
        bool begin_window(std::string_view title);

        /**
         * @brief End window.
         */
        void end_window();

        /**
         * @brief Draw everything.
         */
        void draw(renderer& renderer);

    private:
        /**
         * @brief Draw command.
         */
        struct draw_command {
            /**
             * @brief Texture handle.
             */
            handle texture = null;

            /**
             * @brief Index offset.
             */
            std::size_t index_offset = 0;

            /**
             * @brief Index count.
             */
            std::size_t index_count = 0;

            /**
             * @brief Vertex offset.
             */
            std::size_t vertex_offset = 0;

            /**
             * @brief Vertex count.
             */
            std::size_t vertex_count = 0;
        };

        /**
         * @brief Panel structure.
         */
        struct panel {
            /**
             * @brief Rectangle of the panel.
             */
            vec4 rect = { 0.0f, 24.0f, 256.0f, 512.0f };

            /**
             * @brief Current offset.
             */
            vec2 offset = vec2::zero();

            /**
             * @brief Vertex buffer.
             */
            std::vector<vertex2d> vertices;

            /**
             * @brief Index buffer.
             */
            std::vector<unsigned int> indices;

            /**
             * @brief Draw commands.
             */
            std::vector<draw_command> commands;
        };

        /**
         * @brief Combine two IDs into one.
         * 
         * @return Combined ID.
         */
        std::size_t combine(std::size_t seed, std::size_t id) const;

        /**
         * @brief Draw rect.
         */
        void draw_rect(panel& panel, const vec4& rect, color color);

        /**
         * @brief Draw rect with texture.
         */
        void draw_rect(panel& panel, const texture& texture, const ivec4& src, const vec4& dst, color color);

        /**
         * @brief Draw text.
         */
        void draw_text(panel& panel, const font& font, unsigned char size, std::string_view text, const vec2& position, color color);

        /**
         * @brief Mouse move event.
         */
        void on_mouse_move(const mouse_move_event& event);

        /**
         * @brief Mouse button event.
         */
        void on_mouse_button(const mouse_button_event& event);

        /**
         * @brief Tell whether mouse button was pressed.
         * 
         * @brief True is button was pressed, false otherwise.
         */
        bool is_mouse_button_pressed(mouse_button button);

        /**
         * @brief Window reference.
         */
        window& m_window;

        /**
         * @brief Font reference.
         */
        font& m_font;

        /**
         * @brief Stopwatch to calculate delta time.
         */
        stopwatch m_stopwatch;

        /**
         * @brief Mouse position.
         */
        vec2 m_mouse_position = vec2::zero();

        /**
         * @brief Mouse buttons.
         */
        std::unordered_map<mouse_button, bool> m_mouse_buttons;

        /**
         * @brief Last mouse buttons.
         */
        std::unordered_map<mouse_button, bool> m_last_mouse_buttons;

        /**
         * @brief ID stack.
         */
        std::stack<std::size_t> m_ids;

        /**
         * @brief Panels map.
         */
        std::unordered_map<std::size_t, panel> m_panels;

        /**
         * @brief Current panel ID.
         */
        std::size_t m_panel_id = 0;

        /**
         * @biref Current active ID.
         */
        std::size_t m_active_id = 0;
    };
}
