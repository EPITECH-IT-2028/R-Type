#include "Client.hpp"
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
#include "MovementSystem.hpp"
#include "Packet.hpp"
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
        _sequence_number{0},
        _packet_count{0},
        _ecsManager(ecs::ECSManager::getInstance()),
        _state(ClientState::DISCONNECTED) {
    _running.store(false, std::memory_order_release);
  }

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
   * @brief Registers all component types used by the client with the ECS
   * manager.
   *
   * Registers the following components so they can be attached to entities and
   * queried by systems: PositionComponent, VelocityComponent, RenderComponent,
   * SpriteComponent, ScaleComponent, BackgroundTagComponent,
   * PlayerTagComponent, and SpriteAnimationComponent.
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
   * @brief Create and register an ECS entity representing a player with visual,
   * positional, animation, and identification components.
   *
   * The created entity is configured from values in the provided packet and
   * player sprite configuration, and the entity is recorded in the client's
   * player-entity mapping in a thread-safe manner. If the client's local player
   * ID is not assigned, it is set from the packet and the entity is tagged as
   * the local player.
   *
   * @param packet Packet carrying the player's id and initial position.
   */
  void Client::createPlayerEntity(NewPlayerPacket packet) {
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
    if (_player_id == static_cast<std::uint32_t>(-1)) {
      _player_id = packet.player_id;
      _ecsManager.addComponent<ecs::LocalPlayerTagComponent>(player, {});
    }
    _playerEntities[packet.player_id] = player;
    size_t len = strnlen(packet.player_name, 32);
    _playerName.assign(packet.player_name, len);
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
   * @return Entity The associated entity, or `(Entity)(-1)` if no mapping
   * exists.
   */
  Entity Client::getProjectileEntity(std::uint32_t projectileId) {
    std::lock_guard<std::mutex> lock(_projectileMutex);
    auto it = _projectileEntities.find(projectileId);
    if (it != _projectileEntities.end()) {
      return it->second;
    }
    return static_cast<Entity>(-1);
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
   * @brief Send the local player's input state to the server.
   *
   * If the client has not been assigned a local player ID, the call is ignored.
   *
   * @param input Player input flags encoded as a byte (bitmask).
   */
  void Client::sendInput(std::uint8_t input) {
    if (getPlayerId() == static_cast<std::uint32_t>(-1)) {
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
   * @brief Send a shoot action for the local player to the server at the given
   * world coordinates.
   *
   * If the local player ID is unassigned, no packet is sent and the function
   * returns immediately.
   *
   * @param x World-space X coordinate where the player is shooting.
   * @param y World-space Y coordinate where the player is shooting.
   */
  void Client::sendShoot(float x, float y) {
    if (getPlayerId() == static_cast<std::uint32_t>(-1)) {
      TraceLog(LOG_WARNING,
               "[WARN] Player ID not assigned yet, cannot send shoot");
      return;
    }
    try {
      PlayerShootPacket packet = PacketBuilder::makePlayerShoot(
          x, y, ProjectileType::PLAYER_BASIC,
          _sequence_number.load(std::memory_order_acquire));
      send(packet);
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[SEND SHOOT] Exception: %s", e.what());
    }
  }

  /**
   * @brief Sends a matchmaking request to the connected server.
   *
   * If the request is successfully sent, an informational log entry is
   * produced; if sending fails, an error is logged.
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

  void Client::sendChatMessage(const std::string &message) {
    if (getPlayerId() == static_cast<std::uint32_t>(-1)) {
      TraceLog(
          LOG_WARNING,
          "[SEND CHAT] Player ID not assigned yet, cannot send chat message");
      return;
    }
    try {
      ChatMessagePacket packet =
          PacketBuilder::makeChatMessage(message, getPlayerId());
      send(packet);
    } catch (const std::exception &e) {
      TraceLog(LOG_ERROR, "[SEND CHAT] Exception: %s", e.what());
    }
  }

  void Client::storeChatMessage(const std::string &author, const std::string &message) {
    std::lock_guard<std::mutex> lock(_chatMutex);
    _chatMessages.push_back({author, message});
    if (_chatMessages.size() > CHAT_MAX_MESSAGES)
      _chatMessages.erase(_chatMessages.begin());
  }
}  // namespace client
