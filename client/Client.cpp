#include "Client.hpp"
#include <raylib.h>
#include <atomic>
#include <cstdint>
#include <cstring>
#include "AssetManager.hpp"
#include "BackgroundSystem.hpp"
#include "BackgroundTagComponent.hpp"
#include "ChatComponent.hpp"
#include "Crypto.hpp"
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
#include "PacketLossComponent.hpp"
#include "PingComponent.hpp"
#include "MetricsSystem.hpp"

namespace client {
  /**
   * @brief Constructs a Client configured to connect to the given host and port
   * and starts the background resend thread.
   *
   * Initializes the network manager with the provided host and port, sets the
   * default player name to "Unknown", initializes sequence and packet counters
   * to zero, obtains the ECS manager singleton, sets the client's initial state
   * to DISCONNECTED, starts the resend thread, and sets the running flag to
   * false.
   *
   * @param host Server hostname or IP address.
   * @param port Server port number.
   */
  Client::Client(const std::string &host, const std::uint16_t &port)
      : _networkManager(host, port),
        _playerName("Unknown"),
        _sequence_number{0},
        _packet_count{0},
        _packetLossMonitor(),
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
   * @brief Register all ECS component types required by the client.
   *
   * Makes the following component types available to entities and systems:
   * PositionComponent, VelocityComponent, RenderComponent, SpriteComponent,
   * ScaleComponent, BackgroundTagComponent, PlayerTagComponent,
   * LocalPlayerTagComponent, SpriteAnimationComponent, ProjectileComponent,
   * EnemyComponent, ChatComponent, and PlayerComponent.
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
    _ecsManager.registerComponent<ecs::PingComponent>();
    _ecsManager.registerComponent<ecs::PacketLossComponent>();
    _ecsManager.registerComponent<ecs::PlayerComponent>();
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
    _ecsManager.registerSystem<ecs::MetricsSystem>();
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
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::LocalPlayerTagComponent>());
      _ecsManager.setSystemSignature<ecs::MetricsSystem>(signature);
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
   * @brief Create an ECS player entity from a NewPlayerPacket and register it
   * with the client.
   *
   * Creates a new player entity, attaches the player's components, records the
   * mapping from player ID to entity and player name, prevents duplicate
   * creation for the same player ID, and assigns the local player tag and local
   * player ID when appropriate.
   *
   * @param packet Packet containing the player's ID, null-terminated name, and
   * initial position (x, y).
   */
  void Client::createPlayerEntity(NewPlayerPacket packet) {
    std::unique_lock<std::shared_mutex> lock(_playerStateMutex);

    if (_playerEntities.find(packet.player_id) != _playerEntities.end()) {
      TraceLog(LOG_WARNING,
               "[DUPLICATE PREVENTION] Player entity already exists for "
               "player_id: %u",
               packet.player_id);
      return;
    }

    lock.unlock();

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

    ecs::PlayerComponent playerComp;
    playerComp.player_id = packet.player_id;
    playerComp.name = packet.player_name;
    playerComp.is_alive = true;
    playerComp.connected = true;
    _ecsManager.addComponent<ecs::PlayerComponent>(player, playerComp);

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

    lock.lock();

    bool isLocalPlayer =
        (_player_id == INVALID_ID && packet.player_name == _playerName);

    if (isLocalPlayer) {
      _player_id = packet.player_id;
      _ecsManager.addComponent<ecs::LocalPlayerTagComponent>(player, {});
      TraceLog(LOG_INFO, "[CREATE PLAYER] Set local player ID to %u (%s)",
               _player_id, _playerName.c_str());
    } else if (packet.player_id == _player_id) {
      _ecsManager.addComponent<ecs::LocalPlayerTagComponent>(player, {});
    }
    _ecsManager.addComponent<ecs::PingComponent>(player, {});
    _ecsManager.addComponent<ecs::PacketLossComponent>(player, {});

    _playerEntities[packet.player_id] = player;
    _playerNames[packet.player_id] = packet.player_name;
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
   * If the local player ID is not assigned, the function returns without
   * sending.
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
   * @brief Remove the unacknowledged packet entry for a given sequence number.
   *
   * Erases the stored unacknowledged packet identified by @p sequence_number.
   * If no entry exists for that sequence number, the function has no effect.
   *
   * @param sequence_number Sequence number of the packet to remove.
   */
  void Client::removeAcknowledgedPacket(std::uint32_t sequence_number) {
    std::lock_guard<std::mutex> lock(_unacknowledgedPacketsMutex);
    auto it = _unacknowledged_packets.find(sequence_number);
    if (it != _unacknowledged_packets.end())
      _unacknowledged_packets.erase(it);
  }

  /**
   * @brief Resends unacknowledged packets that are eligible for retransmission.
   *
   * Scans the client's unacknowledged-packet table and resends entries whose
   * last-sent time is older than the minimum resend interval. Each resent entry
   * has its `resend_count` incremented and `last_sent` updated. Entries that
   * reach `MAX_RESEND_ATTEMPTS` are removed and will not be retried.
   *
   * @details
   * - Resent packets are transmitted via the client's network manager.
   * - Constants used: `MIN_RESEND_PACKET_DELAY` (minimum interval) and
   * `MAX_RESEND_ATTEMPTS` (maximum attempts).
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
      MatchmakingRequestPacket packet =
          PacketBuilder::makeMatchmakingRequest(_sequence_number.load());
      send(packet);
      TraceLog(LOG_INFO, "[MATCHMAKING] Sent matchmaking request");
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[MATCHMAKING] Exception: %s", e.what());
    }
  }

  /**
   * @brief Initiates a challenge request for the specified room and sends the
   * corresponding packet to the server.
   *
   * Marks the internal challenge object as waiting for a challenge, constructs
   * a RequestChallengePacket using the current outgoing sequence number, and
   * sends it. If an exception occurs while building or sending the packet, the
   * waiting flag is cleared.
   *
   * @param room_id ID of the room for which to request a challenge.
   */
  void Client::sendRequestChallenge(std::uint32_t room_id) {
    try {
      _challenge.reset();
      _challenge.setRoomId(room_id);
      _challenge.setWaitingChallenge(true);

      RequestChallengePacket packet =
          PacketBuilder::makeRequestChallenge(room_id, _sequence_number.load());
      send(packet);

    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[REQUEST CHALLENGE] Exception: %s", e.what());
      _challenge.setWaitingChallenge(false);
    }
  }

  /**
   * @brief Sends a request to join a room on the server using the provided
   * credentials.
   *
   * The provided plaintext password is hashed with SHA-256 before being sent.
   * If a challenge for the same room has been received, the challenge string is
   * concatenated with the password hash and re-hashed before sending.
   *
   * @param room_id Identifier of the room to join.
   * @param password Plaintext room password; it will be hashed (SHA-256) and
   *                 combined with any pending challenge for the room prior to
   *                 packet construction.
   */
  void Client::sendJoinRoom(std::uint32_t room_id,
                            const std::string &password) {
    try {
      std::string password_hash = crypto::Crypto::sha256(password);

      if (_challenge.isChallengeReceived() &&
          _challenge.getRoomId() == room_id) {
        std::string generateString = _challenge.getChallenge() + password_hash;
        password_hash = crypto::Crypto::sha256(generateString);
      }

      JoinRoomPacket packet = PacketBuilder::makeJoinRoom(
          room_id, password_hash, _sequence_number.load());
      send(packet);
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[JOIN ROOM] Exception: %s", e.what());
    }
  }

  /**
   * @brief Requests creation of a game room on the server.
   *
   * Hashes the provided plaintext password with SHA-256, builds a CreateRoom
   * packet (max players fixed to 4) using the client's current outgoing
   * sequence number, and sends it to the server. Exceptions during packet
   * construction or sending are caught and logged.
   *
   * @param room_name Name of the room to create.
   * @param password Plaintext password for the room; will be hashed with
   * SHA-256 before sending.
   */
  void Client::createRoom(const std::string &room_name,
                          const std::string &password) {
    try {
      auto pwd_hash = crypto::Crypto::sha256(password);
      CreateRoomPacket packet = PacketBuilder::makeCreateRoom(
          room_name, 4, _sequence_number.load(), pwd_hash);
      send(packet);
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[CREATE ROOM] Exception: %s", e.what());
    }
  }

  /**
   * @brief Sends a chat message from the local player to the server.
   *
   * If no local player ID is assigned, the message is not sent.
   *
   * @param message Text of the chat message to send.
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

  void Client::getScoreboard() {
    try {
      ScoreboardRequestPacket packet = PacketBuilder::makeScoreboardRequest();
      send(packet);
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[SCOREBOARD REQUEST] Exception: %s", e.what());
    }
  }
}  // namespace client
