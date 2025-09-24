#include "Game.hpp"
#include <chrono>
#include <iostream>
#include <mutex>
#include "EnemyComponent.hpp"
#include "EntityManager.hpp"
#include "HealthComponent.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "SpeedComponent.hpp"
#include "VelocityComponent.hpp"

game::Game::Game() : _running(false) {
  initECS();
}

void game::Game::initECS() {
  _ecsManager = std::make_unique<ecs::ECSManager>();
  _ecsManager->registerComponent<ecs::PositionComponent>();
  _ecsManager->registerComponent<ecs::HealthComponent>();
  _ecsManager->registerComponent<ecs::SpeedComponent>();
  _ecsManager->registerComponent<ecs::PlayerComponent>();
  _ecsManager->registerComponent<ecs::VelocityComponent>();
  _ecsManager->registerComponent<ecs::EnemyComponent>();

  _enemySystem = _ecsManager->registerSystem<ecs::EnemySystem>();
  _enemySystem->setECSManager(_ecsManager.get());

  Signature enemySignature;
  enemySignature.set(_ecsManager->getComponentType<ecs::EnemyComponent>());
  enemySignature.set(_ecsManager->getComponentType<ecs::PositionComponent>());
  enemySignature.set(_ecsManager->getComponentType<ecs::VelocityComponent>());
  _ecsManager->setSystemSignature<ecs::EnemySystem>(enemySignature);
}

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
}

void game::Game::gameLoop() {
  auto lastTime = std::chrono::high_resolution_clock::now();

  while (_running) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> deltaTime = now - lastTime;
    lastTime = now;

    _enemySystem->update(deltaTime.count());

    spawnEnemy(deltaTime.count());

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

std::shared_ptr<game::Player> game::Game::createPlayer(
    int player_id, const std::string &name) {
  std::scoped_lock lock(_playerMutex);
  auto entity = _ecsManager->createEntity();

  _ecsManager->addComponent<ecs::PositionComponent>(entity, {10.0f, 10.0f});
  _ecsManager->addComponent<ecs::HealthComponent>(entity, {100, 100});
  _ecsManager->addComponent<ecs::SpeedComponent>(entity, {10.0f});
  _ecsManager->addComponent<ecs::PlayerComponent>(entity,
                                                  {name, true, 0, true});
  _ecsManager->addComponent<ecs::VelocityComponent>(entity, {0.0f, 0.0f});

  auto player = std::make_shared<Player>(player_id, entity, _ecsManager.get());
  _players[player_id] = player;

  return player;
}

void game::Game::destroyPlayer(int player_id) {
  std::scoped_lock lock(_playerMutex);
  auto it = _players.find(player_id);
  if (it != _players.end()) {
    uint32_t entity_id = it->second->getEntityId();
    _ecsManager->destroyEntity(entity_id);
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

    auto enemy = createEnemy(_nextEnemyId++, ecs::EnemyType::BASIC);

    if (enemy) {
      std::cout << "[DEBUG] Enemy spawned with ID " << enemy->getEnemyId()
                << std::endl;

      auto pos = enemy->getPosition();
      auto vel = enemy->getVelocity();
      auto health = enemy->getHealth();
      auto max_health = enemy->getMaxHealth();
      queue::EnemySpawnEvent event;
      event.enemy_id = enemy->getEnemyId();
      event.type = ecs::EnemyType::BASIC;
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

std::shared_ptr<game::Enemy> game::Game::createEnemy(
    int enemy_id, const ecs::EnemyType type) {
  std::scoped_lock lock(_enemyMutex);
  auto entity = _ecsManager->createEntity();

  _ecsManager->addComponent<ecs::EnemyComponent>(entity, {type});
  _ecsManager->addComponent<ecs::PositionComponent>(entity, {800.0f, 50.0f});
  _ecsManager->addComponent<ecs::HealthComponent>(entity, {100, 100});
  _ecsManager->addComponent<ecs::VelocityComponent>(entity, {-3.0f, 0.0f});

  auto enemy = std::make_shared<Enemy>(enemy_id, entity, _ecsManager.get());
  _enemies[enemy_id] = enemy;
  return enemy;
}

void game::Game::destroyEnemy(int enemy_id) {
  std::scoped_lock lock(_enemyMutex);
  auto it = _enemies.find(enemy_id);
  if (it != _enemies.end()) {
    uint32_t entity_id = it->second->getEntityId();
    _ecsManager->destroyEntity(entity_id);
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
