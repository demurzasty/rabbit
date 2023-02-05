#include <rabbit/network/client.hpp>

#include "enet.hpp"

using namespace rb;

struct client::data {
    ENetHost* host = nullptr;
    ENetPeer* peer = nullptr;
};

client::client(std::string_view host, unsigned short port)
    : m_data(new data()) {
    enet::retain();

    m_data->host = enet_host_create(nullptr, 1, 2, 0, 0);
    assert(m_data->host);

    ENetAddress address;
    enet_address_set_host(&address, std::string(host).c_str());
    address.port = port;

    m_data->peer = enet_host_connect(m_data->host, &address, 2, 0);
    assert(m_data->peer);
}

client::client(client&& client) noexcept
    : m_data(std::move(client.m_data)) {
}

client::~client() {
    enet_peer_disconnect(m_data->peer, 0);
    enet_peer_reset(m_data->peer);
    enet_host_destroy(m_data->host);

    enet::release();
}

void client::disconnect() {
    enet_peer_disconnect(m_data->peer, 0);
}

void client::send(id_type packet_id, const void* data, std::size_t size) {
    ENetPacket* packet = enet_packet_create(nullptr, size + sizeof(id_type), ENET_PACKET_FLAG_RELIABLE);

    memcpy(packet->data, &packet_id, sizeof(id_type));
    memcpy(packet->data + sizeof(id_type), data, size);

    enet_peer_send(m_data->peer, 0, packet);
}

void client::dispatch() {
    ENetEvent event;
    while (enet_host_service(m_data->host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                m_dispatcher.trigger<client_event_connect>();
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                if (event.packet->dataLength >= sizeof(id_type)) {
                    id_type packet_id;
                    memcpy(&packet_id, event.packet->data, sizeof(id_type));

                    if (auto& handler = m_customs[packet_id]; handler) {
                        handler({ event.packet->data + sizeof(id_type), event.packet->dataLength - sizeof(id_type) }, m_dispatcher);
                    }
                }

                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                m_dispatcher.trigger<client_event_disconnect>();
                break;
        }
    }
}
