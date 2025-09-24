#include "Projectile.hpp"
#include <cstdint>
#include "PositionComponent.hpp"
#include "ProjectileComponent.hpp"
#include "SpeedComponent.hpp"
#include "VelocityComponent.hpp"

game::Projectile::Projectile(std::uint32_t projectile_id,
                             std::uint32_t owner_id,
                             std::uint32_t entity_id,
                             ecs::ECSManager *ecsManager)
    : _projectile_id(projectile_id),
      _owner_id(owner_id),
      _entity_id(entity_id),
      _ecsManager(ecsManager) {
}

std::pair<float, float> game::Projectile::getPosition() const {
  if (hasComponent<ecs::PositionComponent>()) {
    const auto &pos = getComponent<ecs::PositionComponent>();
    return {pos.x, pos.y};
  }
  return {0.0f, 0.0f};
}

void game::Projectile::setPosition(float x, float y) {
  if (hasComponent<ecs::PositionComponent>()) {
    auto &pos = getComponent<ecs::PositionComponent>();
    pos.x = x;
    pos.y = y;
  }
}

void game::Projectile::move(float deltaX, float deltaY) {
  if (hasComponent<ecs::PositionComponent>()) {
    auto &pos = getComponent<ecs::PositionComponent>();
    pos.x += deltaX;
    pos.y += deltaY;
  }
}

bool game::Projectile::isDestroyed() const {
  if (hasComponent<ecs::ProjectileComponent>()) {
    return getComponent<ecs::ProjectileComponent>().is_destroy;
  }
  return false;
}

float game::Projectile::getSpeed() const {
  if (hasComponent<ecs::SpeedComponent>()) {
    return getComponent<ecs::SpeedComponent>().speed;
  }
  return 0.0f;
}

void game::Projectile::setSpeed(float speed) {
  if (hasComponent<ecs::SpeedComponent>()) {
    getComponent<ecs::SpeedComponent>().speed = speed;
  }
}

std::pair<float, float> game::Projectile::getVelocity() const {
  if (hasComponent<ecs::VelocityComponent>()) {
    const auto &vel = getComponent<ecs::VelocityComponent>();
    return {vel.vx, vel.vy};
  }
  return {0.0f, 0.0f};
}

void game::Projectile::setVelocity(float vx, float vy) {
  if (hasComponent<ecs::VelocityComponent>()) {
    auto &vel = getComponent<ecs::VelocityComponent>();
    vel.vx = vx;
    vel.vy = vy;
  }
}

ProjectileType game::Projectile::getType() const {
  if (hasComponent<ecs::ProjectileComponent>()) {
    return getComponent<ecs::ProjectileComponent>().type;
  }
  return ProjectileType{};
}
void game::Projectile::setType(ProjectileType type) {
  if (hasComponent<ecs::ProjectileComponent>()) {
    getComponent<ecs::ProjectileComponent>().type = type;
  }
}

std::uint32_t game::Projectile::getSequenceNumber() const {
  if (hasComponent<ecs::ProjectileComponent>()) {
    return getComponent<ecs::ProjectileComponent>().sequence_number;
  }
  return 0;
}

void game::Projectile::setSequenceNumber(std::uint32_t seq) {
  if (hasComponent<ecs::ProjectileComponent>()) {
    getComponent<ecs::ProjectileComponent>().sequence_number = seq;
  }
}

void game::Projectile::update(float deltaTime) {
  (void)deltaTime;
}
