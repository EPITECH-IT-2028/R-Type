#pragma once

#include <string>
#include <iostream>
#include <asio.hpp>
#include "PacketFactory.hpp"
#include "PacketSender.hpp"

using asio::ip::udp;

namespace client {
  class Client {
    public:
      Client(const std::string &host, const std::string &port)
          : _socket(_io_context), _host(host), _port(port),
            _sequence_number(0), _running(false), _packet_count(0), _packetFactory() {}

      ~Client() = default;
      
      void connect();

      void disconnect();

      template<typename PacketType>
      void send(const PacketType &packet) {
        if (!_running) {
          std::cerr << "Client is not connected. Cannot send packet." << std::endl;
          return;
        }

        try {
          packet::PacketSender::sendPacket(_socket, _server_endpoint, packet);
          _packet_count++;
          _sequence_number++;
        } catch (std::exception &e) {
          std::cerr << "Send error: " << e.what() << std::endl;
        }
      }

      void receivePackets();

      bool isConnected() const { return _running; }

    private:
      asio::io_context _io_context;
      udp::socket _socket;
      udp::endpoint _server_endpoint;
      udp::endpoint _remote_endpoint;
      std::string _host;
      std::string _port;
      std::array<char, 2048> _recv_buffer;
      uint32_t _sequence_number;
      bool _running;
      uint64_t _packet_count;

      packet::PacketHandlerFactory _packetFactory;
  };
}