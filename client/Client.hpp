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

  /**
   * Player sprite and animation configuration.
   * Values must match the layout of the player sprite-sheet used for rendering.
   */
  struct PlayerSpriteConfig {
      static constexpr float RECT_X = 0.0f;  ///< X coordinate in sprite sheet
      static constexpr float RECT_Y = 0.0f;  ///< Y coordinate in sprite sheet
      static constexpr float RECT_WIDTH = 33.0f;   ///< Width of player sprite
      static constexpr float RECT_HEIGHT = 17.0f;  ///< Height of player sprite
      static constexpr int SCALE = 2;          ///< Scale factor for rendering
      static constexpr int TOTAL_COLUMNS = 5;  ///< Columns in sprite sheet
      static constexpr int TOTAL_ROWS = 5;     ///< Rows in sprite sheet
      static constexpr float FRAME_TIME =
          0.05f;  ///< Time per animation frame (seconds)
  };

  /**
   * Frame indices for player animation.
   * These map directly to specific sprite-sheet frames and
   * must match the order/layout of the player sprite asset.
   */
  enum class PlayerSpriteFrameIndex {
    SELECTED_ROW = 0,  ///< Row for basic movement/idle
    NEUTRAL = 2,       ///< Neutral frame index
    END = 4            ///< End frame index (e.g., tilt/extreme)
  };
}  // namespace client

namespace client {
  class Client {
    public:
      Client(const std::string &host, uint16_t port);
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
      asio::io_context _io_context;
      asio::ip::udp::socket _socket;
      asio::ip::udp::endpoint _server_endpoint;
      std::string _host;
      std::uint16_t _port;
      std::array<char, 2048> _recv_buffer;
      std::atomic<uint32_t> _sequence_number;
      std::atomic<bool> _running;
      network::ClientNetworkManager _networkManager;
      std::atomic<uint64_t> _packet_count;
      std::chrono::milliseconds _timeout;

      void registerComponent();
      void registerSystem();
      void signSystem();

      void createBackgroundEntities();
      void createPlayerEntity();

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
