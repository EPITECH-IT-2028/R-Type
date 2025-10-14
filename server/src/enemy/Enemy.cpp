#include "Enemy.hpp"
#include <algorithm>
#include <optional>
#include "EnemyComponent.hpp"
#include "HealthComponent.hpp"
#include "PositionComponent.hpp"
#include "ScoreComponent.hpp"
#include "VelocityComponent.hpp"

/**
 * @brief Retrieve the enemy's position as an (x, y) pair.
 *
 * @return std::pair<float, float> containing the x and y coordinates; returns (0.0f, 0.0f) if the PositionComponent is not present.
 */
std::pair<float, float> game::Enemy::getPosition() const {
  if (hasComponent<ecs::PositionComponent>()) {
    const auto &pos = getComponent<ecs::PositionComponent>();
    return {pos.x, pos.y};
  }
  return {0.0f, 0.0f};
}

void game::Enemy::setPosition(float x, float y) {
  if (hasComponent<ecs::PositionComponent>()) {
    auto &pos = getComponent<ecs::PositionComponent>();
    pos.x = x;
    pos.y = y;
  }
}

void game::Enemy::move(float deltaX, float deltaY) {
  if (hasComponent<ecs::PositionComponent>()) {
    auto &pos = getComponent<ecs::PositionComponent>();
    pos.x += deltaX;
    pos.y += deltaY;
  }
}

std::optional<int> game::Enemy::getHealth() const {
  if (hasComponent<ecs::HealthComponent>()) {
    const auto &health = getComponent<ecs::HealthComponent>();
    return health.health;
  }
  return std::nullopt;
}

std::optional<int> game::Enemy::getMaxHealth() const {
  if (hasComponent<ecs::HealthComponent>()) {
    const auto &health = getComponent<ecs::HealthComponent>();
    return health.max_health;
  }
  return std::nullopt;
}

void game::Enemy::setHealth(int health) {
  if (hasComponent<ecs::HealthComponent>()) {
    auto &healthComp = getComponent<ecs::HealthComponent>();
    const int clampedHealth =
        std::clamp(health, 0, getMaxHealth().value_or(health));
    healthComp.health = clampedHealth;

    if (hasComponent<ecs::EnemyComponent>()) {
      auto &playerComp = getComponent<ecs::EnemyComponent>();
      playerComp.is_alive = clampedHealth > 0;
    }
  }
}

void game::Enemy::takeDamage(int damage) {
  setHealth(getHealth().value_or(0) - damage);
}

void game::Enemy::heal(int amount) {
  setHealth(getHealth().value_or(0) + amount);
}

bool game::Enemy::isAlive() const {
  if (hasComponent<ecs::EnemyComponent>()) {
    return getComponent<ecs::EnemyComponent>().is_alive;
  }
  return false;
}

std::pair<float, float> game::Enemy::getVelocity() const {
  if (hasComponent<ecs::VelocityComponent>()) {
    const auto &vel = getComponent<ecs::VelocityComponent>();
    return {vel.vx, vel.vy};
  }
  return {0.0f, 0.0f};
}

/**
 * @brief Sets the enemy's velocity if a VelocityComponent is present.
 *
 * Updates the component's horizontal and vertical velocity components when available; does nothing if the component is absent.
 *
 * @param vx Horizontal velocity.
 * @param vy Vertical velocity.
 */
void game::Enemy::setVelocity(float vx, float vy) {
  if (hasComponent<ecs::VelocityComponent>()) {
    auto &vel = getComponent<ecs::VelocityComponent>();
    vel.vx = vx;
    vel.vy = vy;
  }
}

/**
 * @brief Retrieves the enemy's score from its ScoreComponent.
 *
 * @return std::uint32_t The score stored in the ScoreComponent, or 0 if the component is not present.
 */
std::uint32_t game::Enemy::getScore() const {
  if (hasComponent<ecs::ScoreComponent>()) {
    return getComponent<ecs::ScoreComponent>().score;
  }
  return 0;
}

/**
 * @brief Updates the enemy's position by integrating its velocity over a time step.
 *
 * If both PositionComponent and VelocityComponent are present, advances the position by
 * vx * deltaTime and vy * deltaTime. If either component is missing, no action is performed.
 *
 * @param deltaTime Time step in seconds used to scale velocity when updating position.
 */
void game::Enemy::update(float deltaTime) {
  if (hasComponent<ecs::PositionComponent>() &&
      hasComponent<ecs::VelocityComponent>()) {
    auto &pos = getComponent<ecs::PositionComponent>();
    const auto &vel = getComponent<ecs::VelocityComponent>();
    pos.x += vel.vx * deltaTime;
    pos.y += vel.vy * deltaTime;
  }
}