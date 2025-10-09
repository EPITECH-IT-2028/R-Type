#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include "ECSManager.hpp"
#include "Packet.hpp"

namespace game {

  class Projectile {
    public:
      Projectile(std::uint32_t projectile_id, std::uint32_t owner_id,
                 std::uint32_t entity_id, ecs::ECSManager &ecsManager);
      ~Projectile() = default;

      std::uint32_t getProjectileId() const {
        return _projectile_id;
      }

      std::uint32_t getEntityId() const {
        return _entity_id;
      }

      std::uint32_t getOwnerId() const {
        return _owner_id;
      }

      std::pair<float, float> getPosition() const;
      void setPosition(float x, float y);
      void move(float deltaX, float deltaY);

      bool isDestroyed() const;

      float getSpeed() const;
      void setSpeed(float speed);

      std::pair<float, float> getVelocity() const;
      void setVelocity(float vx, float vy);

      std::optional<uint32_t> getSequenceNumber() const;
      void setSequenceNumber(std::uint32_t seq);

      ProjectileType getType() const;
      void setType(ProjectileType type);

      std::optional<uint32_t> getDamage() const;
      void setDamage(std::uint32_t damage);

      void update(float deltaTime);

      template <typename T>
      T &getComponent() {
        return _ecsManager.getComponent<T>(_entity_id);
      }

      template <typename T>
      bool hasComponent() const {
        return _ecsManager.hasComponent<T>(_entity_id);
      }

      template <typename T>
      const T &getComponent() const {
        return _ecsManager.getComponent<T>(_entity_id);
      }

    private:
      std::uint32_t _projectile_id;
      std::uint32_t _owner_id;
      std::uint32_t _entity_id;
      ecs::ECSManager &_ecsManager;
  };

}  // namespace game
