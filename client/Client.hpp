#pragma once

#include <sys/stat.h>
#include <cstdint>
#include <unordered_map>
#include "EntityManager.hpp"
#include "Packet.hpp"
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
#include <mutex>
#include <shared_mutex>
#include <string>
#include "ClientNetworkManager.hpp"
#include "ECSManager.hpp"
#include "PacketBuilder.hpp"
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

  struct EnemySpriteConfig {
      static constexpr float RECT_X = 0.0f;  ///< X coordinate in sprite sheet
      static constexpr float RECT_Y = 0.0f;  ///< Y coordinate in sprite sheet
      static constexpr float RECT_WIDTH =
          33.0f;  ///< Width of enemy sprite (1 frame)
      static constexpr float RECT_HEIGHT = 32.0f;  ///< Height of enemy sprite
      static constexpr int SCALE = 1;          ///< Scale factor for rendering
      static constexpr int TOTAL_COLUMNS = 6;  ///< 6 frames horizontally
      static constexpr int TOTAL_ROWS = 1;     ///< 1 row
      static constexpr float FRAME_TIME =
          0.1f;  ///< Time per frame (slower animation)
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

  enum class EnemySpriteFrameIndex {
    SELECTED_ROW = 0,  ///< Row for basic movement/idle
    NEUTRAL = 0,       ///< Starting frame
    END = 2            ///< Last frame (3 frames: 0-2)
  };
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
        if (_player_id == static_cast<std::uint32_t>(-1)) {
          _networkManager.disconnect();
          _running.store(false, std::memory_order_release);
          return;
        }
        PlayerDisconnectPacket packet =
            PacketBuilder::makePlayerDisconnect(_player_id);
        send(packet);
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

      uint32_t getEnemyEntity(uint32_t enemy_id) const {
        auto it = _enemyEntities.find(enemy_id);
        if (it != _enemyEntities.end()) {
          return it->second;
        }
        return KO;
      }

      uint32_t getPlayerEntity(uint32_t player_id) const {
        std::shared_lock<std::shared_mutex> lock(_playerEntitiesMutex);
        auto it = _playerEntities.find(player_id);
        if (it != _playerEntities.end()) {
          return it->second;
        }
        return KO;
      }

      void destroyPlayerEntity(uint32_t playerId) {
        std::lock_guard<std::shared_mutex> lock(_playerEntitiesMutex);
        _playerEntities.erase(playerId);
      }

      void destroyEnemyEntity(uint32_t enemyId) {
        _enemyEntities.erase(enemyId);
      }

      void createPlayerEntity(NewPlayerPacket packet);
      void createEnemyEntity(EnemySpawnPacket packet);

      void addProjectileEntity(uint32_t projectileId, Entity entity);
      Entity getProjectileEntity(uint32_t projectileId);
      void removeProjectileEntity(uint32_t projectileId);

      uint32_t getPlayerId() const {
        return _player_id;
      }

      uint32_t getSequenceNumber() const {
        return _sequence_number.load(std::memory_order_acquire);
      }

      void updateSequenceNumber(uint32_t seq) {
        _sequence_number.store(seq, std::memory_order_release);
      }

      void sendPosition();
      void sendShoot(float x, float y);
      void sendInput(MovementInputType input);

    private:
      std::array<char, 2048> _recv_buffer;
      std::atomic<uint32_t> _sequence_number;
      std::atomic<bool> _running;
      network::ClientNetworkManager _networkManager;
      std::atomic<uint64_t> _packet_count;
      std::chrono::milliseconds _timeout;
      std::unordered_map<uint32_t, Entity> _playerEntities;
      mutable std::shared_mutex _playerEntitiesMutex;
      std::unordered_map<uint32_t, Entity> _enemyEntities;
      std::unordered_map<uint32_t, Entity> _projectileEntities;
      std::mutex _projectileMutex;
      std::uint32_t _player_id = static_cast<std::uint32_t>(-1);

      void registerComponent();
      void registerSystem();
      void signSystem();

      void createBackgroundEntities();

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
