#include "Client.hpp"
#include <cstdint>
#include "AssetManager.hpp"
#include "BackgroundTagComponent.hpp"
#include "BoundarySystem.hpp"
#include "EntityManager.hpp"
#include "InputSystem.hpp"
#include "PlayerTagComponent.hpp"
#include "LocalPlayerTagComponent.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
#include "ScaleComponent.hpp"
#include "SpeedComponent.hpp"
#include "SpriteAnimationComponent.hpp"
#include "SpriteAnimationSystem.hpp"
#include "SpriteComponent.hpp"
#include "VelocityComponent.hpp"
#include "systems/BackgroundSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "EnemyComponent.hpp"
#include "Packet.hpp"

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
    createBackgroundEntities();
  }

  /**
   * @brief Registers all component types used by the client with the ECS
   * manager.
   *
   * Registers the following components so they can be attached to entities and
   * queried by systems: PositionComponent, VelocityComponent, RenderComponent,
   * SpeedComponent, SpriteComponent, ScaleComponent, BackgroundTagComponent,
   * PlayerTagComponent, and SpriteAnimationComponent.
   */
  void Client::registerComponent() {
    _ecsManager.registerComponent<ecs::PositionComponent>();
    _ecsManager.registerComponent<ecs::VelocityComponent>();
    _ecsManager.registerComponent<ecs::RenderComponent>();
    _ecsManager.registerComponent<ecs::SpeedComponent>();
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
   * @brief Registers the game's ECS systems with the ECS manager.
   *
   * Registers the background, movement, input, boundary, sprite animation, and
   * render systems so they are tracked and updated by the ECS manager.
   */
  void Client::registerSystem() {
    _ecsManager.registerSystem<ecs::BackgroundSystem>();
    _ecsManager.registerSystem<ecs::MovementSystem>();
    _ecsManager.registerSystem<ecs::InputSystem>();
    _ecsManager.registerSystem<ecs::BoundarySystem>();
    _ecsManager.registerSystem<ecs::SpriteAnimationSystem>();
    _ecsManager.registerSystem<ecs::ProjectileSystem>();
    _ecsManager.registerSystem<ecs::RenderSystem>();
  }

  /**
   * @brief Assigns component signatures to each ECS system used by the client.
   *
   * Configures which component types each system requires so the ECS manager
   * can match entities to systems. The mappings set here are:
   * - BackgroundSystem: PositionComponent, RenderComponent,
   * BackgroundTagComponent
   * - MovementSystem: PositionComponent, VelocityComponent
   * - RenderSystem: PositionComponent, RenderComponent
   * - InputSystem: VelocityComponent, SpeedComponent, PlayerTagComponent,
   * SpriteAnimationComponent
   * - BoundarySystem: PositionComponent, SpriteComponent, PlayerTagComponent
   * - SpriteAnimationSystem: SpriteComponent, SpriteAnimationComponent
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
      signature.set(_ecsManager.getComponentType<ecs::VelocityComponent>());
      signature.set(_ecsManager.getComponentType<ecs::SpeedComponent>());
      signature.set(_ecsManager.getComponentType<ecs::LocalPlayerTagComponent>());
      signature.set(_ecsManager.getComponentType<ecs::SpriteAnimationComponent>());
      _ecsManager.setSystemSignature<ecs::InputSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::SpriteComponent>());
      signature.set(_ecsManager.getComponentType<ecs::PlayerTagComponent>());
      _ecsManager.setSystemSignature<ecs::BoundarySystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::SpriteComponent>());
      signature.set(_ecsManager.getComponentType<ecs::SpriteAnimationComponent>());
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
    Image backgroundImage = asset::AssetManager::loadImage(renderManager::BG_PATH);
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
   * @brief Creates and configures the player entity in the ECS.
   *
   * Constructs a player entity and attaches its initial components: position,
   * velocity, movement speed, render asset, sprite source rectangle, scale,
   * player tag, and sprite animation metadata.
   *
   * The created entity is positioned at (100, 100) with zero initial velocity
   * and uses renderManager::PLAYER_PATH for rendering. Sprite and scale values
   * are taken from PlayerSpriteConfig. The sprite animation component is
   * initialized with column/row counts, selected/neutral frames, frame timing,
   * and non-playing, non-looping defaults.
   */
  void Client::createPlayerEntity(NewPlayerPacket packet) {
    auto player = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(player, {packet.x, packet.y});
    _ecsManager.addComponent<ecs::VelocityComponent>(player, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::SpeedComponent>(player, {packet.speed});
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
    anim.selectedRow = static_cast<int>(PlayerSpriteFrameIndex::SELECTED_ROW);
    anim.isPlaying = false;
    anim.frameTime = PlayerSpriteConfig::FRAME_TIME;
    anim.loop = false;
    anim.neutralFrame = static_cast<int>(PlayerSpriteFrameIndex::NEUTRAL);
    _ecsManager.addComponent<ecs::SpriteAnimationComponent>(player, anim);

    if (_player_id == -1) {
      _player_id = packet.player_id;
      _ecsManager.addComponent<ecs::LocalPlayerTagComponent>(player, {});
      TraceLog(LOG_INFO, "Assigned player ID: %u", _player_id);
    }
    _playerEntities[packet.player_id] = player;
  }

  void Client::createEnemyEntity(EnemySpawnPacket packet) {
    if (_enemyEntities.find(packet.enemy_id) != _enemyEntities.end()) {
      TraceLog(LOG_WARNING, "[ENEMY SPAWN] Enemy ID: %u already exists", packet.enemy_id);
      return;
    }
    auto enemy = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(enemy, {packet.x, packet.y});
    _ecsManager.addComponent<ecs::VelocityComponent>(enemy, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(
        enemy, {renderManager::ENEMY_PATH});
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

void Client::sendPosition() {
  if (_player_id == static_cast<uint32_t>(-1)) {
    // TraceLog(LOG_WARNING, "[SEND POSITION] Player ID not assigned yet");
    return;
  }

  auto it = _playerEntities.find(_player_id);
  if (it == _playerEntities.end()) {
    TraceLog(LOG_WARNING, "[SEND POSITION] Player entity not found for ID: %u", _player_id);
    return;
  }
  Entity playerEntity = it->second;

  try {
    auto &position = _ecsManager.getComponent<ecs::PositionComponent>(playerEntity);
    
    PositionPacket packet = PacketBuilder::makePosition(
        position.x,
        position.y,
        _sequence_number.load(std::memory_order_acquire));
    
    send(packet);
    
  } catch (const std::exception &e) {
    TraceLog(LOG_ERROR, "[SEND POSITION] Exception: %s", e.what());
  }
}
}  // namespace client
