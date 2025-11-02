#pragma once

#include <raylib.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>
#include "Challenge.hpp"
#include "ClientNetworkManager.hpp"
#include "ECSManager.hpp"
#include "EntityManager.hpp"
#include "Macro.hpp"
#include "PacketBuilder.hpp"
#include "PacketLossMonitor.hpp"
#include "PacketSender.hpp"
#include "PacketUtils.hpp"
#include "Serializer.hpp"

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
      /**
       * @brief Stops the resend thread and joins it during client destruction.
       *
       * Signals the resend thread to stop and joins the thread if it is
       * joinable to ensure the background resend mechanism is terminated before
       * destruction.
       */
      ~Client() {
        _resendThreadRunning.store(false, std::memory_order_release);
        if (_resendThread.joinable()) {
          _resendThread.join();
        }
      }

      /**
       * @brief Reports whether the client has an active network connection.
       *
       * @return `true` if the underlying network manager reports a connection,
       * `false` otherwise.
       */
      bool isConnected() const {
        return _networkManager.isConnected();
      }

      void initializeECS();

      void startReceive() {
        _networkManager.receivePackets();

        _networkManager.processReceivedPackets(*this);
      }

      /**
       * @brief Connects to the server, transitions the client to the connected
       * menu state, and announces the local player.
       *
       * If the connection succeeds, sets the client state to
       * ClientState::IN_CONNECTED_MENU and sends a PlayerInfo packet containing
       * the current player name and the current outgoing sequence number.
       */
      void connect() {
        _networkManager.connect();
        if (isConnected()) {
          setClientState(ClientState::IN_CONNECTED_MENU);

          PlayerInfoPacket packet = PacketBuilder::makePlayerInfo(
              getPlayerName(), _sequence_number.load());
          send(packet);
        }
      }

      /**
       * @brief Disconnects the client from the server and stops its main and
       * resend loops.
       *
       * If a local player ID is assigned, sends a PlayerDisconnect packet using
       * the current outgoing sequence number before closing the network
       * connection. In all cases the network manager is disconnected and the
       * client's running flags are cleared.
       */
      void disconnect() {
        _resendThreadRunning.store(false, std::memory_order_release);

        if (getPlayerId() == static_cast<std::uint32_t>(-1)) {
          _networkManager.disconnect();
          _running.store(false, std::memory_order_release);
          return;
        }
        PlayerDisconnectPacket packet = PacketBuilder::makePlayerDisconnect(
            getPlayerId(), _sequence_number.load());
        send(packet);
        _networkManager.disconnect();
        _running.store(false, std::memory_order_release);
      }

      template <typename PacketType>
      /**
       * @brief Transmit a packet to the server and update sequencing and
       * retransmission bookkeeping.
       *
       * If the client is not connected the function returns without sending. On
       * successful transmission the internal packet counter and outgoing
       * sequence number are advanced. If the packet type requires
       * acknowledgement and carries a sequence number, the serialized packet is
       * recorded for potential resending. Exceptions thrown during send are
       * caught and not propagated.
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
          auto serializedData = std::make_shared<std::vector<uint8_t>>(
              serialization::BitserySerializer::serialize(packet));

          packet::PacketSender::sendPacket(_networkManager, packet);
          ++_packet_count;

          if constexpr (requires { packet.sequence_number; }) {
            if (shouldAcknowledgePacketType(packet.header.type)) {
              addUnacknowledgedPacket(packet.sequence_number, serializedData);
            }
          }
          _sequence_number.fetch_add(1, std::memory_order_release);
        } catch (std::exception &e) {
          std::cerr << "Send error: " << e.what() << std::endl;
        }
      }

      /**
       * @brief Sets the local player's display name.
       *
       * Stores the provided string as the client's local player name, which is
       * used for display and outgoing player-identifying messages.
       *
       * @param name The player name to store.
       */
      void setPlayerName(const std::string &name) {
        std::lock_guard<std::shared_mutex> lock(_playerStateMutex);
        _playerName = name;
        if (_player_id != INVALID_ID)
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
       * @brief Retrieves the local player's identifier.
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
       * Looks up the name mapped to the given playerId. If no mapping exists
       * and playerId equals (std::uint32_t)-1, returns "Server". If no mapping
       * exists for any other id, returns "Unknown".
       *
       * @param playerId Player identifier to look up; the sentinel value
       * `(std::uint32_t)-1` represents the server.
       * @return std::string The player name, "Server" for the sentinel id, or
       * "Unknown" if the id is not found.
       */
      std::string getPlayerNameById(const std::uint32_t playerId) const {
        std::shared_lock<std::shared_mutex> lock(_playerStateMutex);
        auto it = _playerNames.find(playerId);
        if (it != _playerNames.end())
          return it->second;
        if (playerId == INVALID_ID)
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
      void sendRequestChallenge(std::uint32_t room_id);
      void sendJoinRoom(std::uint32_t room_id, const std::string &password);
      void createRoom(const std::string &room_name,
                      const std::string &password);

      void sendChatMessage(const std::string &message);
      void storeChatMessage(const std::string &author,
                            const std::string &message, const Color color);
      /**
       * @brief Get a snapshot of stored chat messages.
       *
       * @return std::vector<ChatMessage> A vector containing a copy of all chat
       * messages as they existed at the time of the call.
       */
      std::vector<ChatMessage> getChatMessages() const {
        std::lock_guard<std::mutex> lock(_chatMutex);
        return _chatMessages;
      }

      /**
       * @brief Retrieves the client's current connection/game state.
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

      double calculatePacketLoss(uint32_t seq) {
        std::lock_guard<std::mutex> lock(_packetLossMutex);
        _packetLossMonitor.onReceived(seq);

        return _packetLossMonitor.lossRatio();
      }

      /**
       * @brief Accesses the client's Challenge instance.
       *
       * @return Challenge& Reference to the client's Challenge object
       * (mutable).
       */
      Challenge &getChallenge() {
        return _challenge;
      }

      void removeAcknowledgedPacket(std::uint32_t sequence_number);

      void getScoreboard();

    private:
      void resendPackets();

      void addUnacknowledgedPacket(
          std::uint32_t sequence_number,
          std::shared_ptr<std::vector<uint8_t>> packetData);

      void resendUnacknowledgedPackets();

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
      std::uint32_t _player_id = INVALID_ID;
      std::string _playerName;
      std::unordered_map<std::uint32_t, std::string> _playerNames;
      mutable std::shared_mutex _playerStateMutex;
      std::vector<ChatMessage> _chatMessages;
      mutable std::mutex _chatMutex;
      std::atomic<ClientState> _state{ClientState::DISCONNECTED};

      std::thread _resendThread;
      mutable std::mutex _unacknowledgedPacketsMutex;
      std::atomic<bool> _resendThreadRunning{false};
      std::unordered_map<std::uint32_t, UnacknowledgedPacket>
          _unacknowledged_packets;

      void registerComponent();
      void registerSystem();
      void signSystem();

      void createBackgroundEntities();

      network::PacketLossMonitor _packetLossMonitor;
      mutable std::mutex _packetLossMutex;

      Challenge _challenge;
      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
