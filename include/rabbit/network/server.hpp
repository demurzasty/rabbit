#pragma once 

#include "../core/reactive.hpp"
#include "../core/type_info.hpp"
#include "../core/span.hpp"

#include <memory>
#include <functional>
#include <unordered_map>

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
         * @brief Disabled copy constructor.
         */
        server(const server&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        server(server&& server) noexcept;

        /**
         * @brief Destruct the server.
         */
        ~server();

        /**
         * @brief Disabled copy assignment.
         */
        server& operator=(const server&) = delete;

        /**
         * @brief Disabled move assignment.
         */
        server& operator=(server&&) = delete;

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
        [[nodiscard]] auto on() { 
            // Handle custom event differently.
            if constexpr (!std::is_same_v<Event, server_event_connect> && !std::is_same_v<Event, server_event_disconnect>) {
                auto& handler = m_customs[type_hash<Event>()];
                if (!handler) {
                    handler = [](span<const unsigned char> data, dispatcher& dispatcher) {
                        if (sizeof(Event) == data.size()) {
                            Event event;
                            memcpy(&event, data.data(), sizeof(Event));

                            dispatcher.trigger(event);
                        }
                    };
                }
            }

            return m_dispatcher.sink<Event>(); 
        }

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

        /**
         * @brief Custom events handler.
         */
        std::unordered_map<id_type, std::function<void(span<const unsigned char>, dispatcher&)>> m_customs;
    };
}