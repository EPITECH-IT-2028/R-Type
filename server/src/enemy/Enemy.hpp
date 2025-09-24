#pragma once

#include <cstdint>
#include "ECSManager.hpp"
namespace game {

  class Enemy {
    public:
      Enemy(int enemy_id, uint32_t entity_id, ecs::ECSManager *ecsManager)
          : _enemy_id(enemy_id),
            _entity_id(entity_id),
            _ecsManager(ecsManager) {
      }
      ~Enemy() = default;

      int getEnemyId() const {
        return _enemy_id;
      }

      uint32_t getEntityId() const {
        return _entity_id;
      }

      std::pair<float, float> getPosition() const;
      void setPosition(float x, float y);
      void move(float deltaX, float deltaY);
      int getHealth() const;
      int getMaxHealth() const;
      void setHealth(int health);
      void takeDamage(int damage);
      void heal(int amount);
      bool isAlive() const;
      std::pair<float, float> getVelocity() const;
      void setVelocity(float vx, float vy);
      void update(float deltaTime);

      template <typename T>
      T &getComponent() {
        return _ecsManager->getComponent<T>(_entity_id);
      }

      template <typename T>
      bool hasComponent() const {
        return _ecsManager->hasComponent<T>(_entity_id);
      }

      template <typename T>
      const T &getComponent() const {
        return _ecsManager->getComponent<T>(_entity_id);
      }

    private:
      int _enemy_id;
      uint32_t _entity_id;
      ecs::ECSManager *_ecsManager;
  };

}  // namespace game
