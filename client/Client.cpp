#include "Client.hpp"
#include <raylib.h>
#include <atomic>
#include <cstdint>
#include <cstring>
#include "AssetManager.hpp"
#include "BackgroundSystem.hpp"
#include "BackgroundTagComponent.hpp"
#include "ChatComponent.hpp"
#include "EnemyComponent.hpp"
#include "EntityManager.hpp"
#include "InputSystem.hpp"
#include "LocalPlayerTagComponent.hpp"
#include "Macro.hpp"
#include "MovementSystem.hpp"
#include "Packet.hpp"
#include "PlayerComponent.hpp"
#include "PlayerTagComponent.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "ProjectileSystem.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
#include "RenderSystem.hpp"
#include "ScaleComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "SpriteAnimationSystem.hpp"
#include "SpriteComponent.hpp"
#include "VelocityComponent.hpp"

namespace client {
  /**
   * @brief Constructs a Client configured to connect to the given host and
   * port.
   *
   * Initializes the network manager, sequence and packet counters, obtains the
   * ECS manager singleton, sets the client's initial state to DISCONNECTED, and
   * marks the running flag as false.
   *
   * @param host Server hostname or IP address.
   * @param port Server port number.
   */
  Client::Client(const std::string &host, const std::uint16_t &port)
      : _networkManager(host, port),
        _playerName("Unknown"),
        _sequence_number{0},
        _packet_count{0},
        _ecsManager(ecs::ECSManager::getInstance()),
        _state(ClientState::DISCONNECTED) {
    _resendThreadRunning.store(true, std::memory_order_release);
    _resendThread = std::thread(&Client::resendPackets, this);
    _running.store(false, std::memory_order_release);
  }

  /**
   * @brief Initializes the entity-component-system for the client.
   *
   * Performs component and system registration, configures system signatures,
   * injects this client instance into the input and render systems (if
   * present), and creates initial world entities such as background layers and
   * the chat UI.
   */
  void Client::initializeECS() {
    registerComponent();
    registerSystem();
    signSystem();

    auto inputSystem = _ecsManager.getSystem<ecs::InputSystem>();
    if (inputSystem)
      inputSystem->setClient(this);
    auto renderSystem = _ecsManager.getSystem<ecs::RenderSystem>();
    if (renderSystem)
      renderSystem->setClient(this);

    createBackgroundEntities();
    createChatMessageUIEntity();
  }

  /**
   * @brief Register all ECS component types used by the client.
   *
   * Makes the following component types available to entities and systems:
   * PositionComponent, VelocityComponent, RenderComponent, SpriteComponent,
   * ScaleComponent, BackgroundTagComponent, PlayerTagComponent,
   * LocalPlayerTagComponent, SpriteAnimationComponent, ProjectileComponent,
   * EnemyComponent, and ChatComponent.
   */
  void Client::registerComponent() {
    _ecsManager.registerComponent<ecs::PositionComponent>();
    _ecsManager.registerComponent<ecs::VelocityComponent>();
    _ecsManager.registerComponent<ecs::RenderComponent>();
    _ecsManager.registerComponent<ecs::SpriteComponent>();
    _ecsManager.registerComponent<ecs::ScaleComponent>();
    _ecsManager.registerComponent<ecs::BackgroundTagComponent>();
    _ecsManager.registerComponent<ecs::PlayerTagComponent>();
    _ecsManager.registerComponent<ecs::LocalPlayerTagComponent>();
    _ecsManager.registerComponent<ecs::SpriteAnimationComponent>();
    _ecsManager.registerComponent<ecs::ProjectileComponent>();
    _ecsManager.registerComponent<ecs::EnemyComponent>();
    _ecsManager.registerComponent<ecs::ChatComponent>();
  }

  /**
   * @brief Registers core ECS systems with the ECS manager.
   *
   * Registers the background, movement, input, sprite animation, projectile,
   * and render systems so they are created and managed by the ECS manager.
   */
  void Client::registerSystem() {
    _ecsManager.registerSystem<ecs::BackgroundSystem>();
    _ecsManager.registerSystem<ecs::MovementSystem>();
    _ecsManager.registerSystem<ecs::InputSystem>();
    _ecsManager.registerSystem<ecs::SpriteAnimationSystem>();
    _ecsManager.registerSystem<ecs::ProjectileSystem>();
    _ecsManager.registerSystem<ecs::RenderSystem>();
  }

  /**
   * @brief Assign ECS component signatures that determine which entities each
   * system processes.
   *
   * Sets the required component types for each system:
   * - BackgroundSystem: PositionComponent, RenderComponent,
   * BackgroundTagComponent
   * - MovementSystem: PositionComponent, VelocityComponent
   * - RenderSystem: PositionComponent, RenderComponent
   * - InputSystem: LocalPlayerTagComponent, SpriteAnimationComponent,
   * PositionComponent
   * - SpriteAnimationSystem: SpriteComponent, SpriteAnimationComponent
   * - ProjectileSystem: PositionComponent, VelocityComponent,
   * ProjectileComponent
   */
  void Client::signSystem() {
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::RenderComponent>());
      signature.set(
          _ecsManager.getComponentType<ecs::BackgroundTagComponent>());
      _ecsManager.setSystemSignature<ecs::BackgroundSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::VelocityComponent>());
      _ecsManager.setSystemSignature<ecs::MovementSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::RenderComponent>());
      _ecsManager.setSystemSignature<ecs::RenderSystem>(signature);
    }
    {
      Signature signature;
      signature.set(
          _ecsManager.getComponentType<ecs::LocalPlayerTagComponent>());
      signature.set(
          _ecsManager.getComponentType<ecs::SpriteAnimationComponent>());
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      _ecsManager.setSystemSignature<ecs::InputSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::SpriteComponent>());
      signature.set(
          _ecsManager.getComponentType<ecs::SpriteAnimationComponent>());
      _ecsManager.setSystemSignature<ecs::SpriteAnimationSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::VelocityComponent>());
      signature.set(_ecsManager.getComponentType<ecs::ProjectileComponent>());
      _ecsManager.setSystemSignature<ecs::ProjectileSystem>(signature);
    }
  }

  /**
   * @brief Creates two scrolling background entities for a continuously tiled
   * backdrop.
   *
   * Loads the background image to determine its aspect ratio and computes a
   * scaled width based on the current screen height, then spawns two entities
   * positioned side-by-side with leftward velocity, render components
   * referencing the background texture, and background tag components.
   */
  void Client::createBackgroundEntities() {
    Image backgroundImage =
        asset::AssetManager::loadImage(renderManager::BG_PATH);
    float screenHeight = GetScreenHeight();
    float aspectRatio = 1.0f;
    float scaledWidth = screenHeight;
    if (backgroundImage.data != nullptr && backgroundImage.height > 0) {
      aspectRatio = static_cast<float>(backgroundImage.width) /
                    static_cast<float>(backgroundImage.height);
      scaledWidth = screenHeight * aspectRatio;
    }

    auto background1 = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(background1, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(
        background1, {-renderManager::SCROLL_SPEED, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(background1,
                                                   {renderManager::BG_PATH});
    _ecsManager.addComponent<ecs::BackgroundTagComponent>(background1, {});

    auto background2 = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(background2,
                                                     {scaledWidth, 0.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(
        background2, {-renderManager::SCROLL_SPEED, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(background2,
                                                   {renderManager::BG_PATH});
    _ecsManager.addComponent<ecs::BackgroundTagComponent>(background2, {});
  }

  /**
   * @brief Create a player entity with position, render, sprite, scale,
   * animation, and identification components.
   *
   * Configures the entity from the provided packet and player sprite
   * configuration, records the entity in the client's player ID → entity and
   * player ID → name mappings, and — if the client has no local player assigned
   * — sets the local player ID, stores the local player name, and tags the
   * entity as the local player.
   *
   * @param packet Packet containing the player's ID, name, and initial position
   * (x, y).
   */
  void Client::createPlayerEntity(NewPlayerPacket packet) {
    std::uint32_t localId = _player_id;
    if (localId != INVALID_ID && packet.player_id == localId) {
      for (auto &entity : _ecsManager.getAllEntities()) {
        if (_ecsManager.hasComponent<ecs::LocalPlayerTagComponent>(entity)) {
          TraceLog(LOG_WARNING,
                   "[DUPLICATE PREVENTION] Local player entity already exists, "
                   "skipping creation for player_id: %u",
                   packet.player_id);
          return;
        }
      }
    }
    for (auto &entity : _ecsManager.getAllEntities()) {
      if (_ecsManager.hasComponent<ecs::PlayerComponent>(entity)) {
        if (_ecsManager.getComponent<ecs::PlayerComponent>(entity).player_id ==
            packet.player_id) {
          return;
        }
      }
    }
    auto player = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(player,
                                                     {packet.x, packet.y});
    _ecsManager.addComponent<ecs::RenderComponent>(
        player, {renderManager::PLAYER_PATH});
    ecs::SpriteComponent sprite;
    sprite.sourceRect = {PlayerSpriteConfig::RECT_X, PlayerSpriteConfig::RECT_Y,
                         PlayerSpriteConfig::RECT_WIDTH,
                         PlayerSpriteConfig::RECT_HEIGHT};
    _ecsManager.addComponent<ecs::SpriteComponent>(player, sprite);
    _ecsManager.addComponent<ecs::ScaleComponent>(
        player, {PlayerSpriteConfig::SCALE, PlayerSpriteConfig::SCALE});
    _ecsManager.addComponent<ecs::PlayerTagComponent>(player, {});
    ecs::SpriteAnimationComponent anim;
    anim.totalColumns = PlayerSpriteConfig::TOTAL_COLUMNS;
    anim.totalRows = PlayerSpriteConfig::TOTAL_ROWS;
    anim.endFrame = static_cast<int>(PlayerSpriteFrameIndex::END);
    anim.selectedRow = packet.player_id % PlayerSpriteConfig::TOTAL_ROWS;
    anim.isPlaying = false;
    anim.frameTime = PlayerSpriteConfig::FRAME_TIME;
    anim.loop = false;
    anim.neutralFrame = static_cast<int>(PlayerSpriteFrameIndex::NEUTRAL);
    _ecsManager.addComponent<ecs::SpriteAnimationComponent>(player, anim);

    std::lock_guard<std::shared_mutex> lock(_playerStateMutex);
    size_t len = strnlen(packet.player_name, sizeof(packet.player_name));
    if (_player_id == INVALID_ID) {
      _player_id = packet.player_id;
      _ecsManager.addComponent<ecs::LocalPlayerTagComponent>(player, {});
      _playerName.assign(packet.player_name, len);

      {
        std::lock_guard<std::mutex> lock(_deferredNewPlayerPacketsMutex);
        for (const auto &deferredPacket : _deferredNewPlayerPackets) {
          if (deferredPacket.player_id != _player_id) {
            createPlayerEntity(deferredPacket);
          }
        }
        _deferredNewPlayerPackets.clear();
      }
    }
    _playerEntities[packet.player_id] = player;
    _playerNames[packet.player_id] = std::string(packet.player_name, len);
  }

  /**
   * @brief Creates and registers an enemy entity from spawn packet data.
   *
   * Creates an ECS entity populated with position, velocity, render, sprite,
   * scale, and animation components, then records the mapping from the packet's
   * enemy_id to the created entity. If an entity with the same enemy_id already
   * exists, logs a warning and returns without creating a new entity.
   *
   * @param packet Spawn packet containing `enemy_id` and initial position `x`,
   * `y`.
   */
  void Client::createEnemyEntity(EnemySpawnPacket packet) {
    if (_enemyEntities.find(packet.enemy_id) != _enemyEntities.end()) {
      TraceLog(LOG_WARNING, "[ENEMY SPAWN] Enemy ID: %u already exists",
               packet.enemy_id);
      return;
    }
    auto enemy = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(enemy,
                                                     {packet.x, packet.y});
    _ecsManager.addComponent<ecs::VelocityComponent>(enemy, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(enemy,
                                                   {renderManager::ENEMY_PATH});
    ecs::SpriteComponent sprite;
    sprite.sourceRect = {EnemySpriteConfig::RECT_X, EnemySpriteConfig::RECT_Y,
                         EnemySpriteConfig::RECT_WIDTH,
                         EnemySpriteConfig::RECT_HEIGHT};
    _ecsManager.addComponent<ecs::SpriteComponent>(enemy, sprite);
    _ecsManager.addComponent<ecs::ScaleComponent>(
        enemy, {EnemySpriteConfig::SCALE, EnemySpriteConfig::SCALE});
    ecs::SpriteAnimationComponent anim;
    anim.totalColumns = EnemySpriteConfig::TOTAL_COLUMNS;
    anim.totalRows = EnemySpriteConfig::TOTAL_ROWS;
    anim.endFrame = static_cast<int>(EnemySpriteFrameIndex::END);
    anim.selectedRow = static_cast<int>(EnemySpriteFrameIndex::SELECTED_ROW);
    anim.isPlaying = false;
    anim.frameTime = EnemySpriteConfig::FRAME_TIME;
    anim.loop = false;
    anim.neutralFrame = static_cast<int>(EnemySpriteFrameIndex::NEUTRAL);
    _ecsManager.addComponent<ecs::SpriteAnimationComponent>(enemy, anim);

    _enemyEntities[packet.enemy_id] = enemy;
  }

  /**
   * @brief Creates an entity that hosts the chat UI.
   *
   * Creates and registers a new ECS entity containing a default-initialized
   * ChatComponent used by the client's chat user interface.
   */
  void Client::createChatMessageUIEntity() {
    auto chatEntity = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::ChatComponent>(chatEntity, {});
  }

  /**
   * @brief Associate a projectile identifier with its ECS entity for later
   * lookup.
   *
   * Stores the mapping in the client's projectile map in a thread-safe manner.
   *
   * @param projectileId Unique identifier for the projectile.
   * @param entity ECS entity corresponding to the projectileId.
   */
  void Client::addProjectileEntity(std::uint32_t projectileId, Entity entity) {
    std::lock_guard<std::mutex> lock(_projectileMutex);
    _projectileEntities[projectileId] = entity;
  }

  /**
   * @brief Retrieves the ECS entity associated with a projectile identifier.
   *
   * @param projectileId Identifier of the projectile to look up.
   * @return Entity The associated entity, or `(Entity)(INVALID_ID)` if no
   * mapping exists.
   */
  Entity Client::getProjectileEntity(std::uint32_t projectileId) {
    std::lock_guard<std::mutex> lock(_projectileMutex);
    auto it = _projectileEntities.find(projectileId);
    if (it != _projectileEntities.end()) {
      return it->second;
    }
    return static_cast<Entity>(INVALID_ID);
  }

  /**
   * @brief Remove the mapping for a projectile by its identifier.
   *
   * Erases the projectileId entry from the client's projectile map in a
   * thread-safe manner. If no entry exists for the given identifier, the
   * function has no effect.
   *
   * @param projectileId Unique identifier of the projectile to remove.
   */
  void Client::removeProjectileEntity(std::uint32_t projectileId) {
    std::lock_guard<std::mutex> lock(_projectileMutex);
    _projectileEntities.erase(projectileId);
  }

  /**
   * @brief Transmit the local player's current input flags to the server.
   *
   * If the client has not been assigned a local player ID, the call is ignored.
   *
   * @param input Bitmask of player input flags (each bit represents an input
   * action).
   */
  void Client::sendInput(std::uint8_t input) {
    if (getPlayerId() == INVALID_ID) {
      TraceLog(LOG_WARNING, "[SEND INPUT] Player ID not assigned yet");
      return;
    }

    try {
      PlayerInputPacket packet = PacketBuilder::makePlayerInput(
          input, _sequence_number.load(std::memory_order_acquire));

      send(packet);

    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[SEND INPUT] Exception: %s", e.what());
    }
  }

  /**
   * @brief Send a player shoot action to the server at the specified world
   * coordinates.
   *
   * If the local player ID is not assigned, this function does nothing.
   * On success, the function builds and transmits a PlayerShootPacket, records
   * the serialized packet as unacknowledged for potential retransmission, and
   * advances the client's outgoing sequence number.
   *
   * @param x World-space X coordinate where the player is shooting.
   * @param y World-space Y coordinate where the player is shooting.
   */
  void Client::sendShoot(float x, float y) {
    if (getPlayerId() == INVALID_ID) {
      TraceLog(LOG_WARNING,
               "[WARN] Player ID not assigned yet, cannot send shoot");
      return;
    }
    try {
      uint32_t currentSeq = _sequence_number.load(std::memory_order_acquire);
      PlayerShootPacket packet = PacketBuilder::makePlayerShoot(
          x, y, ProjectileType::PLAYER_BASIC, currentSeq);
      send(packet);
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[SEND SHOOT] Exception: %s", e.what());
    }
  }

  /**
   * @brief Record a sent packet for retransmission tracking keyed by its
   * sequence number.
   *
   * Creates an unacknowledged-packet entry containing the serialized packet
   * bytes, initializes its resend count to zero, sets its last-sent timestamp
   * to now, and stores it in the client's unacknowledged packet map using the
   * provided sequence number.
   *
   * @param sequence_number Sequence identifier for the packet used as the map
   * key.
   * @param packetData Shared pointer to the serialized packet byte buffer to be
   * resent if unacknowledged.
   */
  void Client::addUnacknowledgedPacket(
      std::uint32_t sequence_number,
      std::shared_ptr<std::vector<uint8_t>> packetData) {
    std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
    UnacknowledgedPacket packet;
    packet.data = packetData;
    packet.resend_count = 0;
    packet.last_sent = std::chrono::steady_clock::now();
    _unacknowledged_packets[sequence_number] = packet;
  }

  /**
   * @brief Remove the tracked unacknowledged packet with the given sequence
   * number.
   *
   * Removes the entry for the acknowledged packet from the client's
   * unacknowledged packet store. If no entry exists for the provided sequence
   * number, a warning is logged and no change is made.
   *
   * @param sequence_number Sequence number of the packet to remove.
   */
  void Client::removeAcknowledgedPacket(std::uint32_t sequence_number) {
    std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
    auto it = _unacknowledged_packets.find(sequence_number);
    if (it != _unacknowledged_packets.end()) {
      TraceLog(LOG_INFO,
               "[ACK] Client removing acknowledged packet %u (had %zu unacked "
               "packets)",
               sequence_number, _unacknowledged_packets.size());
      _unacknowledged_packets.erase(it);
      TraceLog(LOG_INFO,
               "[ACK] Client now has %zu unacknowledged packets remaining",
               _unacknowledged_packets.size());
    } else {
      TraceLog(LOG_WARNING,
               "[ACK] Client tried to remove non-existent packet %u",
               sequence_number);
    }
  }

  /**
   * @brief Retries sending packets that have not been acknowledged.
   *
   * Scans the client's unacknowledged packet table and resends any packet whose
   * last send time is older than the minimum resend interval. Each resend
   * increments the packet's resend count and updates its last-sent timestamp.
   * Packets that reach the maximum resend attempts (5) are removed and will not
   * be retried.
   *
   * @details
   * - Resend interval: 500 milliseconds.
   * - Maximum resend attempts: 5.
   * - Side effects:
   *   - Sends packet data via the client's network manager.
   *   - Updates each packet's `resend_count` and `last_sent`.
   *   - Removes entries that exceeded the maximum resend attempts.
   */
  void Client::resendUnacknowledgedPackets() {
    const auto MIN_RESEND_INTERVAL =
        std::chrono::milliseconds(MIN_RESEND_PACKET_DELAY);
    const auto now = std::chrono::steady_clock::now();

    std::vector<std::shared_ptr<std::vector<uint8_t>>> toSend;
    std::vector<uint32_t> toDrop;
    {
      std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
      for (auto &[seq, packet] : _unacknowledged_packets) {
        if (now - packet.last_sent < MIN_RESEND_INTERVAL)
          continue;
        if (packet.resend_count >= MAX_RESEND_ATTEMPTS) {
          toDrop.push_back(seq);
          continue;
        }
        packet.resend_count++;
        packet.last_sent = now;
        TraceLog(LOG_INFO, "Resending packet %u tried %d of %d", seq,
                 packet.resend_count, MAX_RESEND_ATTEMPTS);
        toSend.push_back(packet.data);
      }
      for (auto seq : toDrop) {
        _unacknowledged_packets.erase(seq);
      }
    }
    for (auto &buf : toSend) {
      _networkManager.send(buf);
    }
  }

  /**
   * @brief Runs the background loop that periodically resends unacknowledged
   * packets.
   *
   * Continuously sleeps for a fixed delay and invokes the resend routine while
   * the resend thread running flag remains set; exits when the running flag is
   * cleared.
   */
  void Client::resendPackets() {
    while (_resendThreadRunning.load(std::memory_order_acquire)) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(RESEND_PACKET_DELAY));

      if (!_resendThreadRunning.load(std::memory_order_acquire))
        break;

      resendUnacknowledgedPackets();
    }
  }

  /**
   * @brief Sends a matchmaking request to the connected server.
   */
  void Client::sendMatchmakingRequest() {
    try {
      MatchmakingRequestPacket packet = PacketBuilder::makeMatchmakingRequest();
      send(packet);
      TraceLog(LOG_INFO, "[MATCHMAKING] Sent matchmaking request");
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[MATCHMAKING] Exception: %s", e.what());
    }
  }

  /**
   * Sends a chat message from the local player to the server.
   *
   * If the local player ID is not assigned, the function logs a warning and
   * does nothing. On failure to build or send the packet, the function logs an
   * error.
   *
   * @param message The text of the chat message to send.
   */
  void Client::sendChatMessage(const std::string &message) {
    if (getPlayerId() == INVALID_ID) {
      TraceLog(
          LOG_WARNING,
          "[SEND CHAT] Player ID not assigned yet, cannot send chat message");
      return;
    }
    try {
      ChatMessagePacket packet = PacketBuilder::makeChatMessage(
          message, getPlayerId(), _sequence_number.load());
      send(packet);
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[SEND CHAT] Exception: %s", e.what());
    }
  }

  /**
   * @brief Appends a chat message to the client's chat history and trims the
   * oldest entries to keep the history at or below CHAT_MAX_MESSAGES.
   *
   * Acquires the internal chat mutex before modifying the stored messages.
   *
   * @param author Name of the message author.
   * @param message Message text to store.
   * @param color Display color for the message.
   */
  void Client::storeChatMessage(const std::string &author,
                                const std::string &message, const Color color) {
    std::lock_guard<std::mutex> lock(_chatMutex);
    _chatMessages.push_back({author, message, color});
    if (_chatMessages.size() > CHAT_MAX_MESSAGES)
      _chatMessages.erase(_chatMessages.begin());
  }
}  // namespace client
