#include "Game.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <thread>
#include "ColliderComponent.hpp"
#include "CollisionSystem.hpp"
#include "EnemyComponent.hpp"
#include "EntityManager.hpp"
#include "Events.hpp"
#include "HealthComponent.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "ProjectileSystem.hpp"
#include "ScoreComponent.hpp"
#include "ShootComponent.hpp"
#include "SpeedComponent.hpp"
#include "VelocityComponent.hpp"

game::Game::Game()
    : _running(false), _ecsManager(ecs::ECSManager::getInstance()) {
  initECS();
  srand(time(nullptr));
}

/**
 * @brief Initialize the game's entity-component-system (ECS) by registering
 * components and configuring core systems.
 *
 * Registers all component types used by the game and creates/configures the
 * core systems, assigning each system the component signature it requires and
 * wiring systems to the game, ECS manager, and event queue.
 *
 * Registered components: Position, Health, Speed, Player, Projectile, Velocity,
 * Enemy, Shoot, Collider, Score.
 *
 * Configured systems and their required signatures:
 * - EnemySystem: Enemy, Position, Velocity, Shoot, Health, Collider
 * - ProjectileSystem: Projectile, Position, Velocity, Collider
 * - CollisionSystem: Position, Collider
 */
void game::Game::initECS() {
  _ecsManager.registerComponent<ecs::PositionComponent>();
  _ecsManager.registerComponent<ecs::HealthComponent>();
  _ecsManager.registerComponent<ecs::SpeedComponent>();
  _ecsManager.registerComponent<ecs::PlayerComponent>();
  _ecsManager.registerComponent<ecs::ProjectileComponent>();
  _ecsManager.registerComponent<ecs::VelocityComponent>();
  _ecsManager.registerComponent<ecs::EnemyComponent>();
  _ecsManager.registerComponent<ecs::ShootComponent>();
  _ecsManager.registerComponent<ecs::ColliderComponent>();
  _ecsManager.registerComponent<ecs::ScoreComponent>();

  _enemySystem = _ecsManager.registerSystem<ecs::EnemySystem>();
  _enemySystem->setECSManager(&_ecsManager);
  _enemySystem->setGame(this);
  _enemySystem->setEventQueue(&_eventQueue);

  _collisionSystem = _ecsManager.registerSystem<ecs::CollisionSystem>();
  _collisionSystem->setGame(this);
  _collisionSystem->setEventQueue(&_eventQueue);

  _projectileSystem = _ecsManager.registerSystem<ecs::ProjectileSystem>();
  _projectileSystem->setECSManager(&_ecsManager);
  Signature enemySignature;
  enemySignature.set(_ecsManager.getComponentType<ecs::EnemyComponent>());
  enemySignature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
  enemySignature.set(_ecsManager.getComponentType<ecs::VelocityComponent>());
  enemySignature.set(_ecsManager.getComponentType<ecs::ShootComponent>());
  enemySignature.set(_ecsManager.getComponentType<ecs::HealthComponent>());
  enemySignature.set(_ecsManager.getComponentType<ecs::ColliderComponent>());
  enemySignature.set(_ecsManager.getComponentType<ecs::ScoreComponent>());
  _ecsManager.setSystemSignature<ecs::EnemySystem>(enemySignature);

  Signature projectileSignature;
  projectileSignature.set(
      _ecsManager.getComponentType<ecs::ProjectileComponent>());
  projectileSignature.set(
      _ecsManager.getComponentType<ecs::PositionComponent>());
  projectileSignature.set(
      _ecsManager.getComponentType<ecs::VelocityComponent>());
  projectileSignature.set(
      _ecsManager.getComponentType<ecs::ColliderComponent>());
  _ecsManager.setSystemSignature<ecs::ProjectileSystem>(projectileSignature);

  Signature collisionSignature;
  collisionSignature.set(
      _ecsManager.getComponentType<ecs::PositionComponent>());
  collisionSignature.set(
      _ecsManager.getComponentType<ecs::ColliderComponent>());
  _ecsManager.setSystemSignature<ecs::CollisionSystem>(collisionSignature);
}

/**
 * @brief Stops the game loop and cleans up the game thread.
 *
 * Ensures the running flag is cleared so the main loop stops, then joins the
 * internal game thread if it is joinable to guarantee the thread has finished
 * before destruction.
 */
game::Game::~Game() {
  stop();
  if (_gameThread.joinable()) {
    _gameThread.join();
  }
}

void game::Game::start() {
  if (_running) {
    return;
  }
  _running = true;
  _gameThread = std::thread(&Game::Game::gameLoop, this);
}

void game::Game::stop() {
  _running = false;
  if (_gameThread.joinable()) {
    _gameThread.join();
  }
}

/**
 * @brief Executes the game's main loop, updating core systems and spawning
 * enemies until stopped.
 *
 * Runs while the Game's running flag remains set. Each iteration computes the
 * elapsed frame time (in seconds) and passes it to the enemy, projectile, and
 * collision systems, invokes enemy spawn logic, and sleeps briefly to cap the
 * frame rate.
 */
void game::Game::gameLoop() {
  std::this_thread::sleep_for(std::chrono::seconds(10));

  queue::GameStartEvent startEvent;
  startEvent.game_started = true;
  _eventQueue.addRequest(startEvent);

  auto lastTime = std::chrono::high_resolution_clock::now();

  while (_running) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTime = now - lastTime;
    lastTime = now;

    _enemySystem->update(deltaTime.count());
    _projectileSystem->update(deltaTime.count());
    _collisionSystem->update(deltaTime.count());

    spawnEnemy(deltaTime.count());

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

/**
 * @brief Create a new player entity, attach initial components, and register
 * the player.
 *
 * Creates an ECS entity for the player, attaches Position, Health, Speed,
 * Player, Velocity, Shoot, Collider, and Score components, stores the resulting
 * Player instance in the game's player registry, and returns it.
 *
 * @param player_id Unique identifier for the player.
 * @param name Player display name.
 * @return std::shared_ptr<game::Player> Shared pointer to the created Player
 * stored in the game.
 */
std::shared_ptr<game::Player> game::Game::createPlayer(
    std::uint32_t player_id, const std::string &name) {
  std::scoped_lock lock(_playerMutex);
  auto entity = _ecsManager.createEntity();

  _ecsManager.addComponent<ecs::PositionComponent>(entity, {10.0f, 10.0f});
  _ecsManager.addComponent<ecs::HealthComponent>(entity, {100, 100});
  _ecsManager.addComponent<ecs::SpeedComponent>(entity, {PLAYER_SPEED});
  _ecsManager.addComponent<ecs::PlayerComponent>(
      entity, {player_id, name, true, 0, true});
  _ecsManager.addComponent<ecs::VelocityComponent>(entity, {0.0f, 0.0f});
  _ecsManager.addComponent<ecs::ShootComponent>(entity,
                                                {0.0f, 3.0f, true, 0.0f});
  ecs::ColliderComponent collider;
  collider.center = {25.f, 25.f};
  collider.halfSize = {25.f, 25.f};
  _ecsManager.addComponent<ecs::ColliderComponent>(entity, collider);
  _ecsManager.addComponent<ecs::ScoreComponent>(entity, {0});

  auto player = std::make_shared<Player>(player_id, entity, _ecsManager);
  _players[player_id] = player;

  return player;
}

void game::Game::destroyPlayer(int player_id) {
  std::scoped_lock lock(_playerMutex);
  auto it = _players.find(player_id);
  if (it != _players.end()) {
    uint32_t entity_id = it->second->getEntityId();
    _ecsManager.destroyEntity(entity_id);
    _players.erase(it);
  }
}

std::shared_ptr<game::Player> game::Game::getPlayer(int player_id) {
  std::scoped_lock lock(_playerMutex);
  auto it = _players.find(player_id);
  return (it != _players.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<game::Player>> game::Game::getAllPlayers() const {
  std::scoped_lock lock(_playerMutex);
  std::vector<std::shared_ptr<Player>> playerList;
  playerList.reserve(_players.size());
  for (const auto &pair : _players) {
    playerList.push_back(pair.second);
  }
  return playerList;
}

void game::Game::spawnEnemy(float deltaTime) {
  _enemySpawnTimer += deltaTime;

  if (_enemySpawnTimer >= _enemySpawnInterval) {
    _enemySpawnTimer = 0.0f;

    auto enemy = createEnemy(_nextEnemyId++, EnemyType::BASIC_FIGHTER);

    if (enemy) {
      auto pos = enemy->getPosition();
      auto vel = enemy->getVelocity();
      auto health = enemy->getHealth().value_or(0);
      auto max_health = enemy->getMaxHealth().value_or(0);
      queue::EnemySpawnEvent event;
      event.enemy_id = enemy->getEnemyId();
      event.type = EnemyType::BASIC_FIGHTER;
      event.x = pos.first;
      event.y = pos.second;
      event.vx = vel.first;
      event.vy = vel.second;
      event.health = health;
      event.max_health = max_health;
      _eventQueue.addRequest(event);
    }
  }
}

/**
 * @brief Create and register an enemy with the specified id and type.
 *
 * Creates an enemy entity, configures its gameplay components according to the
 * given EnemyType, stores the resulting Enemy instance in the game's enemy
 * registry, and returns a shared pointer to that Enemy.
 *
 * @param enemy_id Unique identifier assigned to the new enemy.
 * @param type EnemyType value that determines the enemy's configuration and
 * behavior.
 * @return std::shared_ptr<Enemy> Shared pointer to the created Enemy, or
 * `nullptr` if the provided EnemyType is unsupported.
 */
std::shared_ptr<game::Enemy> game::Game::createEnemy(int enemy_id,
                                                     const EnemyType type) {
  std::scoped_lock lock(_enemyMutex);
  uint32_t entity;
  switch (type) {
    case EnemyType::BASIC_FIGHTER: {
      std::scoped_lock ecsLock(_ecsMutex);
      entity = _ecsManager.createEntity();

      float spawnY =
          static_cast<float>(rand() % ENEMY_SPAWN_Y + ENEMY_SPAWN_OFFSET);
      float spawnX = ENEMY_SPAWN_X;

      _ecsManager.addComponent<ecs::EnemyComponent>(entity, {enemy_id, type});
      _ecsManager.addComponent<ecs::PositionComponent>(entity,
                                                       {spawnX, spawnY});
      _ecsManager.addComponent<ecs::HealthComponent>(entity, {100, 100});
      _ecsManager.addComponent<ecs::VelocityComponent>(entity,
                                                       {ENEMY_SPEED, 0.0f});
      _ecsManager.addComponent<ecs::ShootComponent>(entity,
                                                    {0.0f, 3.0f, true, 0.0f});
      ecs::ColliderComponent collider;
      collider.center = {25.f, 25.f};
      collider.halfSize = {25.f, 30.f};
      _ecsManager.addComponent<ecs::ColliderComponent>(entity, collider);
      _ecsManager.addComponent<ecs::ScoreComponent>(entity, {10});
      break;
    }
    default:
      return nullptr;
  }

  auto enemy = std::make_shared<Enemy>(enemy_id, entity, _ecsManager);
  _enemies[enemy_id] = enemy;
  return enemy;
}

void game::Game::destroyEnemy(int enemy_id) {
  std::scoped_lock lock(_enemyMutex);
  auto it = _enemies.find(enemy_id);
  if (it != _enemies.end()) {
    uint32_t entity_id = it->second->getEntityId();
    _ecsManager.destroyEntity(entity_id);
    _enemies.erase(it);
  }
}

std::shared_ptr<game::Enemy> game::Game::getEnemy(int enemy_id) {
  std::scoped_lock lock(_enemyMutex);
  auto it = _enemies.find(enemy_id);
  return (it != _enemies.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<game::Enemy>> game::Game::getAllEnemies() const {
  std::scoped_lock lock(_enemyMutex);
  std::vector<std::shared_ptr<Enemy>> enemyList;
  enemyList.reserve(_enemies.size());
  for (const auto &pair : _enemies) {
    enemyList.push_back(pair.second);
  }
  return enemyList;
}

/**
 * @brief Create and register a new projectile entity, store it in the game's
 * projectile registry, and emit a spawn event.
 *
 * @param projectile_id Unique identifier for the projectile.
 * @param owner_id Identifier of the entity that owns or fired the projectile.
 * @param type Projectile type.
 * @param x Initial X position of the projectile.
 * @param y Initial Y position of the projectile.
 * @param vx Initial X velocity of the projectile.
 * @param vy Initial Y velocity of the projectile.
 * @return std::shared_ptr<game::Projectile> Shared pointer to the created
 * Projectile.
 */
std::shared_ptr<game::Projectile> game::Game::createProjectile(
    std::uint32_t projectile_id, std::uint32_t owner_id, ProjectileType type,
    float x, float y, float vx, float vy) {
  std::shared_ptr<Projectile> projectile;
  uint32_t entity;
  {
    std::scoped_lock ecsLock(_ecsMutex);
    entity = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(entity, {x, y});
    _ecsManager.addComponent<ecs::SpeedComponent>(entity, {10.0f});
    _ecsManager.addComponent<ecs::ProjectileComponent>(
        entity, {projectile_id, type, owner_id, false,
                 (type == ProjectileType::ENEMY_BASIC), 10, 0, 100});
    _ecsManager.addComponent<ecs::VelocityComponent>(entity, {vx, vy});
    _ecsManager.addComponent<ecs::ColliderComponent>(entity, {10.f, 10.f});
    projectile = std::make_shared<Projectile>(projectile_id, owner_id, entity,
                                              _ecsManager);
  }
  {
    std::scoped_lock lk(_projectileMutex);
    _projectiles[projectile_id] = projectile;
  }

  queue::ProjectileSpawnEvent event;
  event.projectile_id = projectile_id;
  event.owner_id = owner_id;
  event.type = type;
  event.x = x;
  event.y = y;
  event.damage = projectile->getDamage().value_or(0);
  event.is_enemy_projectile = (type == ProjectileType::ENEMY_BASIC);
  event.vx = vx;
  event.vy = vy;
  _eventQueue.addRequest(event);
  return projectile;
}

void game::Game::destroyProjectile(std::uint32_t projectile_id) {
  std::scoped_lock lock(_projectileMutex);
  auto it = _projectiles.find(projectile_id);
  if (it != _projectiles.end()) {
    uint32_t entity_id = it->second->getEntityId();
    {
      std::scoped_lock ecsLock(_ecsMutex);
      _ecsManager.destroyEntity(entity_id);
    }
    _projectiles.erase(it);
  }
}

std::shared_ptr<game::Projectile> game::Game::getProjectile(
    std::uint32_t projectile_id) {
  std::scoped_lock lock(_projectileMutex);
  auto it = _projectiles.find(projectile_id);
  return (it != _projectiles.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<game::Projectile>> game::Game::getAllProjectiles()
    const {
  std::scoped_lock lock(_projectileMutex);
  std::vector<std::shared_ptr<Projectile>> projectileList;
  projectileList.reserve(_projectiles.size());
  for (const auto &pair : _projectiles) {
    projectileList.push_back(pair.second);
  }
  return projectileList;
}
