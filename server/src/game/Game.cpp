#include "Game.hpp"
#include <chrono>
#include <cstdint>
#include <mutex>
#include <thread>
#include "HealthComponent.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
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
  _ecsManager->registerComponent<ecs::ProjectileComponent>();
  _ecsManager->registerComponent<ecs::VelocityComponent>();
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
  while (_running) {
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

std::shared_ptr<game::Projectile> game::Game::createProjectile(
    std::uint16_t projectile_id, uint32_t owner_id, const ProjectileType &type, float x, float y) {
  std::scoped_lock lock(_projectileMutex);
  auto entity = _ecsManager->createEntity();

  _ecsManager->addComponent<ecs::PositionComponent>(entity, {x, y});
  _ecsManager->addComponent<ecs::SpeedComponent>(entity, {10.0f});
  _ecsManager->addComponent<ecs::ProjectileComponent>(entity, {type});
  _ecsManager->addComponent<ecs::VelocityComponent>(entity, {0.0f, 0.0f});

  auto projectile =
      std::make_shared<Projectile>(projectile_id, entity, _ecsManager.get());
  _projectiles[projectile_id] = projectile;

  return projectile;
}

void game::Game::destroyProjectile(std::uint16_t projectile_id) {
  std::scoped_lock lock(_projectileMutex);
  auto it = _projectiles.find(projectile_id);
  if (it != _projectiles.end()) {
    uint32_t entity_id = it->second->getProjectileId();
    _ecsManager->destroyEntity(entity_id);
    _projectiles.erase(it);
  }
}

std::shared_ptr<game::Projectile> game::Game::getProjectile(std::uint16_t projectile_id) {
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
