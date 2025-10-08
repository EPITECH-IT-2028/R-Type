#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include "ECSManager.hpp"
#include "PacketFactory.hpp"
#include "PacketSender.hpp"

#if defined(_WIN32)
    #define PLATFORM_DESKTOP
    #define NOGDI
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
    #define _WIN32_WINNT 0x0601
#endif
#include <asio.hpp>

#define TIMEOUT_MS 100

namespace client {
  constexpr int OK = 0;
  constexpr int KO = 1;
  constexpr float PLAYER_SPEED = 500.0f;
}  // namespace client

namespace client {
  class Client {
    public:
      Client(const std::string &host, const std::string &port);
      ~Client() = default;

      bool isConnected() const {
        return _running.load(std::memory_order_acquire);
      }

      void connect();
      void disconnect();
      void receivePackets();
      void initializeECS();

      template <typename PacketType>
      void send(const PacketType &packet) {
        if (!_running.load(std::memory_order_acquire)) {
          std::cerr << "Client is not connected. Cannot send packet."
                    << std::endl;
          return;
        }

        try {
          packet::PacketSender::sendPacket(_socket, _server_endpoint, packet);
          ++_packet_count;
          ++_sequence_number;
        } catch (std::exception &e) {
          std::cerr << "Send error: " << e.what() << std::endl;
        }
      }

    private:
      asio::io_context _io_context;
      asio::ip::udp::socket _socket;
      asio::ip::udp::endpoint _server_endpoint;
      std::string _host;
      std::string _port;
      std::array<char, 2048> _recv_buffer;
      std::atomic<uint32_t> _sequence_number;
      std::atomic<bool> _running;
      std::atomic<uint64_t> _packet_count;
      std::chrono::milliseconds _timeout;

      packet::PacketHandlerFactory _packetFactory;

      void registerComponent();
      void registerSystem();
      void signSystem();

      void createBackgroundEntities();
      void createPlayerEntity();

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
