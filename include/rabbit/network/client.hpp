#pragma once 

#include "../core/reactive.hpp"

#include <memory>
#include <vector>
#include <string_view>

namespace rb {
    /**
     * @brief Connected to the server event.
     */
    struct client_event_connect { };

    /**
     * @brief Disconnected from the server event.
     */
    struct client_event_disconnect { };

    /**
     * @brief Packet from server has been delivered.
     */
    struct client_event_packet { 
        /**
         * @brief Packet data sent by the server.
         */
        std::vector<unsigned char> data;
    };

    /**
     * @brief Client class.
     */
    class client {
    public:
        /**
         * @brief Construct a new client.
         *
         * @param host Host to connect.
         * @param port Port to connect.
         */
        client(std::string_view host, unsigned short port);

        /**
         * @brief Destruct the client.
         */
        ~client();

        /**
         * @brief Disconnect from the server.
         */
        void disconnect();

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
