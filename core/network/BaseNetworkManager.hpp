#pragma once

#include <asio.hpp>
#include <array>
#include <functional>
#include "Macros.hpp"

namespace network {

class BaseNetworkManager {
public:
    explicit BaseNetworkManager(std::uint16_t port)
        : _socket(_io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)) {}

    virtual ~BaseNetworkManager() = default;

    virtual void startReceive(const std::function<void(const char*, std::size_t)>& callback) = 0;
    virtual void send(const char* data, std::size_t size) = 0;

    virtual void run() = 0;
    virtual void stop() = 0; 

protected:
    asio::io_context _io_context;
    asio::ip::udp::socket _socket;
    std::array<char, BUFFER_SIZE> _recv_buffer;
};

} // namespace net

