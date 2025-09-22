#include "Game.hpp"
#include "HealthComponent.hpp"
#include "NetworkComponent.hpp"
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
  _ecsManager->registerComponent<ecs::NetworkComponent>();
  _ecsManager->registerComponent<ecs::PlayerComponent>();
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
  auto entity = _ecsManager->createEntity();

  _ecsManager->addComponent<ecs::PositionComponent>(entity, {10.0f, 10.0f});
  _ecsManager->addComponent<ecs::HealthComponent>(entity, {100});
  _ecsManager->addComponent<ecs::SpeedComponent>(entity, {10.0f});
  _ecsManager->addComponent<ecs::NetworkComponent>(entity,
                                                   {player_id, true, 0});
  _ecsManager->addComponent<ecs::PlayerComponent>(entity,
                                                  {name, 100, true, 0, true});
  _ecsManager->addComponent<ecs::VelocityComponent>(entity, {0.0f, 0.0f});

  auto player = std::make_shared<Player>(player_id, entity, _ecsManager.get());
  _players[player_id] = player;

  return player;
}

void game::Game::destroyPlayer(int player_id) {
  auto it = _players.find(player_id);
  if (it != _players.end()) {
    uint32_t entity_id = it->second->getEntityId();
    _ecsManager->destroyEntity(entity_id);
    _players.erase(it);
  }
}

std::shared_ptr<game::Player> game::Game::getPlayer(int player_id) {
  auto it = _players.find(player_id);
  return (it != _players.end()) ? it->second : nullptr;
}

const std::vector<std::shared_ptr<game::Player>> &game::Game::getAllPlayers()
    const {
  static std::vector<std::shared_ptr<Player>> playerList;
  playerList.clear();
  for (const auto &pair : _players) {
    playerList.push_back(pair.second);
  }
  return playerList;
}
