#pragma once 

#include "../core/reactive.hpp"
#include "../core/type_info.hpp"
#include "../core/span.hpp"

#include <memory>
#include <functional>
#include <string_view>
#include <type_traits>

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
         * @brief Disabled copy constructor.
         */
        client(const client&) = delete;

        /**
         * @brief Enabled move constructor.
         */
        client(client&& client) noexcept;

        /**
         * @brief Destruct the client.
         */
        ~client();

        /**
         * @brief Disabled copy assignment.
         */
        client& operator=(const client&) = delete;

        /**
         * @brief Disabled move assignment.
         */
        client& operator=(client&&) = delete;

        /**
         * @brief Disconnect from the server.
         */
        void disconnect();

        /**
         * @brief Sending data to the server.
         */
        template<typename T, std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
        void send(const T& data) {
            send(type_hash<T>(), &data, sizeof(T));
        };

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
            if constexpr (!std::is_same_v<Event, client_event_connect> && !std::is_same_v<Event, client_event_disconnect>) {
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
         * @brief Sending raw-data to the server.
         */
        void send(id_type packet_id, const void* data, std::size_t size);

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
