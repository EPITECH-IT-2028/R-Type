#include "Enemy.hpp"
#include <algorithm>
#include "EnemyComponent.hpp"
#include "HealthComponent.hpp"
#include "Macros.hpp"
#include "PositionComponent.hpp"
#include "VelocityComponent.hpp"

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

int game::Enemy::getHealth() const {
  if (hasComponent<ecs::HealthComponent>()) {
    const auto &health = getComponent<ecs::HealthComponent>();
    return health.health;
  }
  return SUCCESS;
}

int game::Enemy::getMaxHealth() const {
  if (hasComponent<ecs::HealthComponent>()) {
    const auto &health = getComponent<ecs::HealthComponent>();
    return health.max_health;
  }
  return SUCCESS;
}

void game::Enemy::setHealth(int health) {
  if (hasComponent<ecs::HealthComponent>()) {
    auto &healthComp = getComponent<ecs::HealthComponent>();
    const int clampedHealth = std::clamp(health, 0, getMaxHealth());
    healthComp.health = clampedHealth;

    if (hasComponent<ecs::EnemyComponent>()) {
      auto &playerComp = getComponent<ecs::EnemyComponent>();
      playerComp.is_alive = clampedHealth > 0;
    }
  }
}

void game::Enemy::takeDamage(int damage) {
  setHealth(getHealth() - damage);
}

void game::Enemy::heal(int amount) {
  setHealth(getHealth() + amount);
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

void game::Enemy::setVelocity(float vx, float vy) {
  if (hasComponent<ecs::VelocityComponent>()) {
    auto &vel = getComponent<ecs::VelocityComponent>();
    vel.vx = vx;
    vel.vy = vy;
  }
}

void game::Enemy::update(float deltaTime) {
  if (hasComponent<ecs::PositionComponent>() &&
      hasComponent<ecs::VelocityComponent>()) {
    auto &pos = getComponent<ecs::PositionComponent>();
    const auto &vel = getComponent<ecs::VelocityComponent>();
    pos.x += vel.vx * deltaTime;
    pos.y += vel.vy * deltaTime;
  }
}
