#pragma once 

#include "events.hpp"
#include "../math/vec2.hpp"
#include "../core/reactive.hpp"

#include <memory>
#include <string_view>

namespace rb {
    /**
     * @brief Window base class.
     */
    class window {
    public:
        /**
         * @brief Construct a new window.
         * 
         * @param title Title of the window.
         * @param size Size of the rendering area of the window.
         * @param fullscreen Specify that the window should be in full screen mode.
         */
        window(std::string_view title, const uvec2& size, bool fullscreen);

        /**
         * @brief Disabled copy constructor.
         */
        window(const window&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        window(window&&) noexcept = default;

        /**
         * @brief Destructor of the window.
         */
        virtual ~window();

        /**
         * @brief Disabled copy assignment.
         */
        window& operator=(const window&) = delete;

        /**
         * @brief Enabled move assignment.
         */
        window& operator=(window&&) noexcept = default;

        /**
         * @brief Close the window.
         * 
         * @remarks After calling this function, the window instance remains
         *          valid.
         */
        void close();

        /**
         * @brief Tell whether of not the window is open.
         * 
         * @return True if the window is open, false if it has been closed.
         */
        [[nodiscard]] bool is_open() const;

        /**
         * @brief Call the event handlers for each pending event.
         */
        void dispatch_events();

        /**
         * @brief Wait for a new event and dispatch it to the corresponding event handler.
         */
        void wait_dispatch_events();

        /**
         * @brief Get the window event sink.
         * 
         * @tparam Event Type of event of which to get the sink.
         * 
         * @return Event sink of the window.
         */
        template<typename Event>
        [[nodiscard]] auto on() { return m_dispatcher.sink<Event>(); }

        /**
         * @brief Change the title of the window.
         * 
         * @param title New window title.
         */
        void set_title(std::string_view title);

        /**
         * @brief Get the position of the window.
         * 
         * @return Position of the window in pixels.
         */
        [[nodiscard]] ivec2 position() const;

        /**
         * @brief Get the size of the window.
         * 
         * @return Size of the window in pixels.
         */
        [[nodiscard]] uvec2 size() const;

        /**
         * @brief Get the OS-specific handle of the window.
         * 
         * @return System handle of the window.
         */
        [[nodiscard]] void* handle() const;

    private:
        /**
         * @brief Opaque pllatform-specific implementation data structure.
         */
        struct data;

        /** 
         * @brief Platform-specific implementation data of the window.
         */
        std::unique_ptr<data> m_data;

        /**
         * @brief Window events dispatcher.
         */
        dispatcher m_dispatcher;
    };
}
