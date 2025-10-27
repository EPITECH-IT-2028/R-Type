#pragma once

#include "Macro.hpp"
#if defined(_WIN32)
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif

  #define PLATFORM_DESKTOP
  #define NOGDI
  #define NOUSER
  #define _WINSOCK_DEPRECATED_NO_WARNINGS
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <raylib.h>
#include <sys/stat.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "ClientNetworkManager.hpp"
#include "ECSManager.hpp"
#include "EntityManager.hpp"
#include "Packet.hpp"
#include "PacketBuilder.hpp"
#include "PacketSender.hpp"

#define TIMEOUT_MS 100

namespace client {
  constexpr int OK = 0;
  constexpr int KO = 1;
  constexpr std::size_t CHAT_MAX_MESSAGES = 14;

  enum class ClientState {
    IN_CONNECTED_MENU,
    IN_ROOM_WAITING,
    IN_GAME,
    DISCONNECTED
  };

  struct ChatMessage {
      std::string author;
      std::string message;
      Color color;
  };

  /**
   * Player sprite and animation configuration.
   * Values must match the layout of the player sprite-sheet used for
   * rendering.
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
        _networkManager.receivePackets();

        _networkManager.processReceivedPackets(*this);
      }

      /**
       * @brief Connects to the server, transitions the client to the connected menu state, and announces the local player.
       *
       * Initiates a network connection; if the connection is established, sets the client state to ClientState::IN_CONNECTED_MENU
       * and sends a PlayerInfo packet containing the current local player name.
       */
      void connect() {
        _networkManager.connect();
        if (isConnected()) {
          setClientState(ClientState::IN_CONNECTED_MENU);

          PlayerInfoPacket packet = PacketBuilder::makePlayerInfo(_playerName);
          send(packet);
        }
      }

      /**
       * @brief Disconnects the client from the server and stops the client's
       * main loop.
       *
       * If the local player ID is valid, a PlayerDisconnect packet for that
       * player is sent before the network connection is closed. In all cases
       * the network manager is disconnected and the running flag is cleared.
       */
      void disconnect() {
        if (getPlayerId() == static_cast<std::uint32_t>(-1)) {
          _networkManager.disconnect();
          _running.store(false, std::memory_order_release);
          return;
        }
        PlayerDisconnectPacket packet =
            PacketBuilder::makePlayerDisconnect(getPlayerId());
        send(packet);
        _networkManager.disconnect();
        _running.store(false, std::memory_order_release);
      }

      template <typename PacketType>
      /**
       * @brief Sends a packet to the server using the client's network manager.
       *
       * If the client is not connected, the call returns without sending. On
       * successful transmission the client's internal packet counter and
       * outgoing sequence number are incremented. Exceptions thrown during send
       * are caught and not propagated.
       *
       * @tparam PacketType Type of the packet to send.
       * @param packet Packet to transmit over the network.
       */
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

      /**
       * @brief Sets the local player's display name.
       *
       * Stores the provided string as the client's local player name, which is used for display and outgoing player-identifying messages.
       *
       * @param name The player name to store.
       */
      void setPlayerName(const std::string &name) {
        std::lock_guard<std::shared_mutex> lock(_playerStateMutex);
        _playerName = name;
        if (_player_id != static_cast<std::uint32_t>(INVALID_ID))
          _playerNames[_player_id] = name;
      }

      /**
       * @brief Retrieve the ECS entity ID associated with an enemy identifier.
       *
       * @returns std::uint32_t The entity ID mapped to the given enemy_id, or
       * `KO` if no mapping exists.
       */
      std::uint32_t getEnemyEntity(std::uint32_t enemy_id) const {
        auto it = _enemyEntities.find(enemy_id);
        if (it != _enemyEntities.end()) {
          return it->second;
        }
        return KO;
      }

      /**
       * @brief Retrieves the ECS entity associated with a player ID.
       *
       * @param player_id The player identifier to look up.
       * @return std::uint32_t The entity ID mapped to the given player, or `KO`
       * if no mapping exists.
       */
      std::uint32_t getPlayerEntity(std::uint32_t player_id) const {
        std::shared_lock<std::shared_mutex> lock(_playerStateMutex);
        auto it = _playerEntities.find(player_id);
        if (it != _playerEntities.end()) {
          return it->second;
        }
        return KO;
      }

      /**
       * @brief Removes the entity mapping for the specified player ID.
       *
       * @param playerId Player identifier whose entity mapping will be removed.
       */
      void destroyPlayerEntity(std::uint32_t playerId) {
        std::lock_guard<std::shared_mutex> lock(_playerStateMutex);
        _playerEntities.erase(playerId);
      }

      /**
       * @brief Remove the enemy entity associated with the given enemy ID from
       * the client's entity map.
       *
       * If no entity is mapped for the provided ID, no action is taken.
       *
       * @param enemyId The enemy identifier whose entity should be removed.
       */
      void destroyEnemyEntity(std::uint32_t enemyId) {
        _enemyEntities.erase(enemyId);
      }

      void createPlayerEntity(NewPlayerPacket packet);
      void createEnemyEntity(EnemySpawnPacket packet);
      void createChatMessageUIEntity();

      void addProjectileEntity(std::uint32_t projectileId, Entity entity);
      Entity getProjectileEntity(std::uint32_t projectileId);
      void removeProjectileEntity(std::uint32_t projectileId);

      /**
       * @brief Retrieve the local player's identifier.
       *
       * @return std::uint32_t The local player ID; returns `(std::uint32_t)-1`
       * if no player is assigned.
       */
      std::uint32_t getPlayerId() const {
        std::shared_lock<std::shared_mutex> lock(_playerStateMutex);
        return _player_id;
      }

      /**
       * @brief Retrieves the local client's current player name.
       *
       * This accessor is thread-safe.
       *
       * @return std::string The local player's name; empty string if not set.
       */
      std::string getPlayerName() const {
        std::shared_lock<std::shared_mutex> lock(_playerStateMutex);
        return _playerName;
      }

      /**
       * @brief Retrieve the display name associated with a player identifier.
       *
       * Looks up the name mapped to the given playerId. If no mapping exists and
       * playerId equals (std::uint32_t)-1, returns "Server". If no mapping exists
       * for any other id, returns "Unknown".
       *
       * @param playerId Player identifier to look up; the sentinel value `(std::uint32_t)-1` represents the server.
       * @return std::string The player name, "Server" for the sentinel id, or "Unknown" if the id is not found.
       */
      std::string getPlayerNameById(const std::uint32_t playerId) const {
        std::shared_lock<std::shared_mutex> lock(_playerStateMutex);
        auto it = _playerNames.find(playerId);
        if (it != _playerNames.end())
          return it->second;
        if (playerId == static_cast<std::uint32_t>(-1))
          return "Server";
        return "Unknown";
      }

      /**
       * @brief Retrieves the current outgoing packet sequence number.
       *
       * @return Current outgoing packet sequence number.
       */
      std::uint32_t getSequenceNumber() const {
        return _sequence_number.load(std::memory_order_acquire);
      }

      /**
       * @brief Update the client's outgoing packet sequence number.
       *
       * Sets the sequence number to use for subsequent outgoing packets.
       *
       * @param seq Sequence number to set for future outgoing packets.
       */
      void updateSequenceNumber(std::uint32_t seq) {
        _sequence_number.store(seq, std::memory_order_release);
      }

      void sendInput(std::uint8_t input);
      void sendShoot(float x, float y);
      void sendMatchmakingRequest();

      void sendChatMessage(const std::string &message);
      void storeChatMessage(const std::string &author,
                            const std::string &message, const Color color);
      /**
       * @brief Get a snapshot of stored chat messages.
       *
       * @return std::vector<ChatMessage> A vector containing a copy of all chat messages as they existed at the time of the call.
       */
      std::vector<ChatMessage> getChatMessages() const {
        std::lock_guard<std::mutex> lock(_chatMutex);
        return _chatMessages;
      }

      /**
       * @brief Retrieve the client's current connection/game state.
       *
       * @return ClientState The current client state.
       */
      ClientState getClientState() const {
        return _state.load(std::memory_order_acquire);
      }

      /**
       * @brief Update the client's current state.
       *
       * Atomically sets the client's state to the provided value.
       *
       * @param state New client state to apply.
       */
      void setClientState(ClientState state) {
        _state.store(state, std::memory_order_release);
      }

    private:
      std::array<char, 2048> _recv_buffer;
      std::atomic<std::uint32_t> _sequence_number;
      std::atomic<bool> _running;
      network::ClientNetworkManager _networkManager;
      std::atomic<std::uint64_t> _packet_count;
      std::chrono::milliseconds _timeout;
      std::unordered_map<std::uint32_t, Entity> _playerEntities;
      std::unordered_map<std::uint32_t, Entity> _enemyEntities;
      std::unordered_map<std::uint32_t, Entity> _projectileEntities;
      std::mutex _projectileMutex;
      std::uint32_t _player_id = static_cast<std::uint32_t>(-1);
      std::string _playerName;
      std::unordered_map<std::uint32_t, std::string> _playerNames;
      mutable std::shared_mutex _playerStateMutex;
      std::vector<ChatMessage> _chatMessages;
      mutable std::mutex _chatMutex;
      std::atomic<ClientState> _state{ClientState::DISCONNECTED};

      void registerComponent();
      void registerSystem();
      void signSystem();

      void createBackgroundEntities();

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client