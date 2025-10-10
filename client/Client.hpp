#pragma once

#include <cstdint>
#if defined(_WIN32)
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #ifndef ASIO_NO_WIN32_LEAN_AND_MEAN
    #define ASIO_NO_WIN32_LEAN_AND_MEAN
  #endif
  #define PLATFORM_DESKTOP
  #define NOGDI
  #define NOUSER
  #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <array>
#include <asio.hpp>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include "ClientNetworkManager.hpp"
#include "ECSManager.hpp"
#include "PacketSender.hpp"

#define TIMEOUT_MS 100

namespace client {
  constexpr int OK = 0;
  constexpr int KO = 1;
  constexpr float PLAYER_SPEED = 500.0f;
}  // namespace client

namespace client {
  class Client {
    public:
      Client(const std::string &host, const std::uint16_t &port);
      ~Client() = default;

      bool isConnected() const {
        return _networkManager.isConnected();
      }

      void initializeECS();
      void startReceive() {
        _networkManager.receivePackets(*this);
      }
      void connect() {
        _networkManager.connect();
      }
      void disconnect() {
        _networkManager.disconnect();
        _running.store(false, std::memory_order_release);
      }

      template <typename PacketType>
      void send(const PacketType &packet) {
        if (!isConnected()) {
          std::cerr << "Client is not connected. Cannot send packet."
                    << std::endl;
          return;
        }

        try {
          packet::PacketSender::sendPacket(_networkManager, packet);
          ++_packet_count;
          ++_sequence_number;
        } catch (std::exception &e) {
          std::cerr << "Send error: " << e.what() << std::endl;
        }
      }

    private:
      network::ClientNetworkManager _networkManager;
      std::atomic<uint64_t> _packet_count;
      std::chrono::milliseconds _timeout;
      std::atomic<uint32_t> _sequence_number;
      std::atomic<bool> _running;
      std::array<char, 2048> _recv_buffer;

      void registerComponent();
      void registerSystem();
      void signSystem();

      void createBackgroundEntities();
      void createPlayerEntity();

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
