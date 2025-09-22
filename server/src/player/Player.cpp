#include "Player.hpp"
#include <algorithm>
#include "HealthComponent.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "SpeedComponent.hpp"
#include "VelocityComponent.hpp"

game::Player::Player(int player_id, uint32_t entity_id,
                     ecs::ECSManager *ecsManager)
    : _player_id(player_id), _entity_id(entity_id), _ecsManager(ecsManager) {
}

template <typename T>
T &game::Player::getComponent() {
  return _ecsManager->getComponent<T>(_entity_id);
}

template <typename T>
bool game::Player::hasComponent() const {
  return _ecsManager->hasComponent<T>(_entity_id);
}

template <typename T>
const T &game::Player::getComponent() const {
  return _ecsManager->getComponent<T>(_entity_id);
}

std::pair<float, float> game::Player::getPosition() const {
  if (hasComponent<ecs::PositionComponent>()) {
    const auto &pos = getComponent<ecs::PositionComponent>();
    return {pos.x, pos.y};
  }
  return {0.0f, 0.0f};
}

void game::Player::setPosition(float x, float y) {
  if (hasComponent<ecs::PositionComponent>()) {
    auto &pos = getComponent<ecs::PositionComponent>();
    pos.x = x;
    pos.y = y;
  }
}

void game::Player::move(float deltaX, float deltaY) {
  if (hasComponent<ecs::PositionComponent>()) {
    auto &pos = getComponent<ecs::PositionComponent>();
    pos.x += deltaX;
    pos.y += deltaY;
  }
}

int game::Player::getHealth() const {
  if (hasComponent<ecs::PlayerComponent>()) {
    const auto &player = getComponent<ecs::PlayerComponent>();
    return player.is_alive ? player.max_health : 0;
  }
  return 0;
}

int game::Player::getMaxHealth() const {
  if (hasComponent<ecs::PlayerComponent>()) {
    const auto &player = getComponent<ecs::PlayerComponent>();
    return player.max_health;
  }
  return 0;
}

void game::Player::setHealth(int health) {
  if (hasComponent<ecs::HealthComponent>()) {
    auto &healthComp = getComponent<ecs::HealthComponent>();
    healthComp.health = std::clamp(health, 0, getMaxHealth());

    if (hasComponent<ecs::PlayerComponent>()) {
      auto &playerComp = getComponent<ecs::PlayerComponent>();
      playerComp.is_alive = (health > 0);
    }
  }
}

void game::Player::takeDamage(int damage) {
  setHealth(getHealth() - damage);
}

void game::Player::heal(int amount) {
  setHealth(getHealth() + amount);
}

bool game::Player::isAlive() const {
  if (hasComponent<ecs::PlayerComponent>()) {
    return getComponent<ecs::PlayerComponent>().is_alive;
  }
  return getHealth() > 0;
}

float game::Player::getSpeed() const {
  if (hasComponent<ecs::SpeedComponent>()) {
    return getComponent<ecs::SpeedComponent>().speed;
  }
  return 0.0f;
}

void game::Player::setSpeed(float speed) {
  if (hasComponent<ecs::SpeedComponent>()) {
    getComponent<ecs::SpeedComponent>().speed = speed;
  }
}

std::pair<float, float> game::Player::getVelocity() const {
  if (hasComponent<ecs::VelocityComponent>()) {
    const auto &vel = getComponent<ecs::VelocityComponent>();
    return {vel.vx, vel.vy};
  }
  return {0.0f, 0.0f};
}

void game::Player::setVelocity(float vx, float vy) {
  if (hasComponent<ecs::VelocityComponent>()) {
    auto &vel = getComponent<ecs::VelocityComponent>();
    vel.vx = vx;
    vel.vy = vy;
  }
}

uint32_t game::Player::getSequenceNumber() const {
  if (hasComponent<ecs::PlayerComponent>()) {
    return getComponent<ecs::PlayerComponent>().sequence_number;
  }
  return 0;
}

void game::Player::setSequenceNumber(uint32_t seq) {
  if (hasComponent<ecs::PlayerComponent>()) {
    getComponent<ecs::PlayerComponent>().sequence_number = seq;
  }
}

bool game::Player::isConnected() const {
  if (hasComponent<ecs::PlayerComponent>()) {
    return getComponent<ecs::PlayerComponent>().connected;
  }
  return false;
}

void game::Player::setConnected(bool connected) {
  if (hasComponent<ecs::PlayerComponent>()) {
    getComponent<ecs::PlayerComponent>().connected = connected;
  }
}

const std::string &game::Player::getName() const {
  if (hasComponent<ecs::PlayerComponent>()) {
    return getComponent<ecs::PlayerComponent>().name;
  }
  return NULL;
}

void game::Player::setName(const std::string &name) {
  if (hasComponent<ecs::PlayerComponent>()) {
    getComponent<ecs::PlayerComponent>().name = name;
  }
}

void game::Player::update(float deltaTime) {
  (void)deltaTime;
}
