#include "Player.hpp"
#include <algorithm>
#include <optional>
#include "HealthComponent.hpp"
#include "PlayerComponent.hpp"
#include "PositionComponent.hpp"
#include "SpeedComponent.hpp"
#include "VelocityComponent.hpp"

/**
 * @brief Initialize a Player with identifiers and an ECS manager reference.
 *
 * @param player_id Integer identifier for the player.
 * @param entity_id Entity identifier associated with this player.
 * @param ecsManager Reference to the ECS manager used to access and modify
 * components for this player.
 */
game::Player::Player(int player_id, std::uint32_t entity_id,
                     ecs::ECSManager &ecsManager)
    : _player_id(player_id), _entity_id(entity_id), _ecsManager(ecsManager) {
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

std::optional<int> game::Player::getHealth() const {
  if (hasComponent<ecs::HealthComponent>()) {
    return getComponent<ecs::HealthComponent>().health;
  }
  return std::nullopt;
}

std::optional<int> game::Player::getMaxHealth() const {
  if (hasComponent<ecs::HealthComponent>()) {
    return getComponent<ecs::HealthComponent>().max_health;
  }
  return std::nullopt;
}

void game::Player::setHealth(int health) {
  if (hasComponent<ecs::HealthComponent>()) {
    auto &healthComp = getComponent<ecs::HealthComponent>();
    const int clampedHealth =
        std::clamp(health, 0, getMaxHealth().value_or(health));
    healthComp.health = clampedHealth;

    if (hasComponent<ecs::PlayerComponent>()) {
      auto &playerComp = getComponent<ecs::PlayerComponent>();
      playerComp.is_alive = clampedHealth > 0;
    }
  }
}

void game::Player::takeDamage(int damage) {
  setHealth(getHealth().value_or(0) - damage);
}

void game::Player::heal(int amount) {
  setHealth(getHealth().value_or(0) + amount);
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

/**
 * @brief Sets the entity's velocity components when a VelocityComponent is
 * present.
 *
 * Updates the VelocityComponent's horizontal and vertical velocity values.
 *
 * @param vx Velocity along the X axis.
 * @param vy Velocity along the Y axis.
 */
void game::Player::setVelocity(float vx, float vy) {
  if (hasComponent<ecs::VelocityComponent>()) {
    auto &vel = getComponent<ecs::VelocityComponent>();
    vel.vx = vx;
    vel.vy = vy;
  }
}

/**
 * @brief Retrieve the player's network sequence number if available.
 *
 * @return std::optional<std::uint32_t> The player's sequence number when a
 * PlayerComponent is present, otherwise `std::nullopt`.
 */
std::optional<std::uint32_t> game::Player::getSequenceNumber() const {
  if (hasComponent<ecs::PlayerComponent>()) {
    return getComponent<ecs::PlayerComponent>().sequence_number;
  }
  return std::nullopt;
}

/**
 * @brief Set the player's sequence number if a PlayerComponent is attached.
 *
 * Does nothing if the entity does not have a PlayerComponent.
 *
 * @param seq Sequence number to assign to the player.
 */
void game::Player::setSequenceNumber(std::uint32_t seq) {
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
  static const std::string empty = "";
  return empty;
}

void game::Player::setName(const std::string &name) {
  if (hasComponent<ecs::PlayerComponent>()) {
    getComponent<ecs::PlayerComponent>().name = name;
  }
}

void game::Player::update(float deltaTime) {
  (void)deltaTime;
}