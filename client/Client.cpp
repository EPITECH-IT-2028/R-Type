#include "Client.hpp"
#include <cstdint>
#include "AssetManager.hpp"
#include "BackgroundTagComponent.hpp"
#include "EnemyComponent.hpp"
#include "EntityManager.hpp"
#include "InputSystem.hpp"
#include "LocalPlayerTagComponent.hpp"
#include "Packet.hpp"
#include "PlayerTagComponent.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
#include "ScaleComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "SpriteAnimationSystem.hpp"
#include "SpriteComponent.hpp"
#include "VelocityComponent.hpp"
#include "systems/BackgroundSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/RenderSystem.hpp"

namespace client {
  Client::Client(const std::string &host, const std::uint16_t &port)
      : _networkManager(host, port),
        _sequence_number{0},
        _packet_count{0},
        _ecsManager(ecs::ECSManager::getInstance()) {
    _running.store(false, std::memory_order_release);
  }

  void Client::initializeECS() {
    registerComponent();
    registerSystem();
    signSystem();

    auto inputSystem = _ecsManager.getSystem<ecs::InputSystem>();
    if (inputSystem)
      inputSystem->setClient(this);

    createBackgroundEntities();
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
   * @brief Configure required component signatures for each ECS system used by
   * the client.
   *
   * Sets which component types entities must have to be processed by each
   * system:
   * - BackgroundSystem: PositionComponent, RenderComponent,
   * BackgroundTagComponent
   * - MovementSystem: PositionComponent, VelocityComponent
   * - RenderSystem: PositionComponent, RenderComponent
   * - InputSystem: VelocityComponent, LocalPlayerTagComponent,
   * SpriteAnimationComponent
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
   * @brief Create a player entity with visual, movement, and identification
   * components.
   *
   * Attaches a PositionComponent (from packet.x/packet.y), a zeroed
   * VelocityComponent, a RenderComponent using the player render asset, a
   * SpriteComponent and ScaleComponent using PlayerSpriteConfig values, and a
   * configured SpriteAnimationComponent. Also attaches a PlayerTagComponent. If
   * the client's local player ID is unassigned, assigns it from
   * packet.player_id and attaches a LocalPlayerTagComponent. Records the
   * created entity in the client's player-entity mapping in a thread-safe
   * manner.
   *
   * @param packet NewPlayerPacket containing the player's id, initial position,
   * and speed.
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

    if (_player_id == static_cast<uint32_t>(-1)) {
      _player_id = packet.player_id;
      _ecsManager.addComponent<ecs::LocalPlayerTagComponent>(player, {});
    }
    std::lock_guard<std::shared_mutex> lock(_playerEntitiesMutex);
    _playerEntities[packet.player_id] = player;
  }

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

  void Client::addProjectileEntity(uint32_t projectileId, Entity entity) {
    std::lock_guard<std::mutex> lock(_projectileMutex);
    _projectileEntities[projectileId] = entity;
  }

  Entity Client::getProjectileEntity(uint32_t projectileId) {
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
  void Client::removeProjectileEntity(uint32_t projectileId) {
    std::lock_guard<std::mutex> lock(_projectileMutex);
    _projectileEntities.erase(projectileId);
  }

  /**
   * @brief Sends the local player's input state to the server.
   *
   * If the client has not been assigned a local player ID, the call is ignored.
   * Any exceptions raised while building or sending the packet are caught and
   * not propagated.
   *
   * @param input The player's input state encoded as a byte (input flags).
   */
  void Client::sendInput(uint8_t input) {
    if (_player_id == static_cast<std::uint32_t>(-1)) {
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
    if (_player_id == static_cast<uint32_t>(-1)) {
      TraceLog(LOG_WARNING,
               "[WARN] Player ID not assigned yet, cannot send "
               "shoot");
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
}  // namespace client
