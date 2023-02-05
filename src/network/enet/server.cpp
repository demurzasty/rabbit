#include <rabbit/network/server.hpp>

#include "enet.hpp"

using namespace rb;

struct server::data {
    ENetHost* host = nullptr;
};

server::server(unsigned short port)
    : m_data(new data()) {
    enet::retain();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    m_data->host = enet_host_create(&address, 32, 2, 0, 0);
    assert(m_data->host);
}

server::~server() {
    enet_host_destroy(m_data->host);

    enet::release();
}

void server::dispatch() {
    ENetEvent event;
    while (enet_host_service(m_data->host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                m_dispatcher.trigger<server_event_connect>();
                break;
            case ENET_EVENT_TYPE_RECEIVE: {
                if (event.packet->dataLength >= sizeof(id_type)) {
                    id_type packet_id;
                    memcpy(&packet_id, event.packet->data, sizeof(id_type));

                    if (auto& handler = m_customs[packet_id];  handler) {
                        handler({ event.packet->data + sizeof(id_type), event.packet->dataLength - sizeof(id_type) }, m_dispatcher);
                    }
                }

                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
                m_dispatcher.trigger<server_event_disconnect>();
                break;
        }
    }
}
