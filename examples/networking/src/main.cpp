#include <rabbit/rabbit.hpp>

using namespace rb;

static bool can_stop = false;

void on_client_connected(client& client) {
    println("connected");

    client.disconnect();
}

void on_client_disconnected() {
    println("disconnected");

    can_stop = true;
}

void start_server() {
    server server(6969);

    server.on<server_event_disconnect>().connect<&on_client_disconnected>();

    while (!can_stop) {
        server.dispatch();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void start_client() {
    client client("127.0.0.1", 6969);

    client.on<client_event_connect>().connect<&on_client_connected>(client);

    while (!can_stop) {
        client.dispatch();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main(int argc, char* argv[]) {
    std::thread server(&start_server);
    std::thread client(&start_client);

    if (client.joinable()) {
        client.join();
    }

    if (server.joinable()) {
        server.join();
    }
}
