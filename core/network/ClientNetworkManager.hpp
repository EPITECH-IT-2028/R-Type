#pragma once

#include "BaseNetworkManager.hpp"

namespace network {

class ClientNetworkManager : public BaseNetworkManager {
public:
    ClientNetworkManager(const std::string& host, std::uint16_t port);

    void startReceive(const std::function<void(const char*, std::size_t)>& callback) override;
    void send(const char* data, std::size_t size) override;

private:
    asio::ip::udp::endpoint _server_endpoint;
    asio::ip::udp::endpoint _remote_endpoint;
};

} // namespace net

