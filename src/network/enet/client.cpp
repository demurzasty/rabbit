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

client::~client() {
    enet_peer_disconnect(m_data->peer, 0);
    enet_peer_reset(m_data->peer);
    enet_host_destroy(m_data->host);

    enet::release();
}

void client::disconnect() {
    enet_peer_disconnect(m_data->peer, 0);
}

void client::dispatch() {
    ENetEvent event;
    while (enet_host_service(m_data->host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                m_dispatcher.enqueue<client_event_connect>();
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                m_dispatcher.enqueue<client_event_disconnect>();
                break;
        }
    }

    m_dispatcher.update();
}
