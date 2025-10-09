#include "ClientNetworkManager.hpp"

using namespace network;

ClientNetworkManager::ClientNetworkManager(const std::string& host, std::uint16_t port)
    : BaseNetworkManager(0),
      _server_endpoint(asio::ip::make_address(host), port) {}

void ClientNetworkManager::startReceive(const std::function<void(const char*, std::size_t)>& callback) {
    _socket.async_receive_from(asio::buffer(_recv_buffer), _remote_endpoint,
        [this, callback](const asio::error_code &error, std::size_t bytes_transferred) {
            if (!error && bytes_transferred > 0) {
                callback(_recv_buffer.data(), bytes_transferred);
            }
            startReceive(callback);
        });
}

void ClientNetworkManager::send(const char* data, std::size_t size) {
    _socket.async_send_to(asio::buffer(data, size), _server_endpoint, [](auto, auto){});
}

