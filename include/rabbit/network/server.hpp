#pragma once 

#include "../core/reactive.hpp"

#include <memory>

namespace rb {
    /**
     * @brief Client has been connected to the server.
     */
    struct server_event_connect { };

    /**
     * @brief Client has been disconnected to the server.
     */
    struct server_event_disconnect { };

    /**
     * @brief Server class.
     */
    class server {
    public:
        /**
         * @brief Construct a new server.
         * 
         * @param port Port to listen.
         */
        server(unsigned short port);

        /**
         * @brief Destruct the server.
         */
        ~server();

        /**
         * @brief Dispatch network events.
         */
        void dispatch();

        /**
         * @brief Get the window event sink.
         *
         * @tparam Event Type of event of which to get the sink.
         *
         * @return Event sink of the window.
         */
        template<typename Event>
        [[nodiscard]] auto on() { return m_dispatcher.sink<Event>(); }

    private:
        /**
         * @brief Implementation specific data structure.
         */
        struct data;

        /**
         * @brief Implementation specific data pointer.
         */
        std::unique_ptr<data> m_data;

        /**
         * @brief Event dispatcher.
         */
        dispatcher m_dispatcher;
    };
}