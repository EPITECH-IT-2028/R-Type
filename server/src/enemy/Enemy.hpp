#pragma once

#include <cstdint>
#include <optional>
#include "ECSManager.hpp"
namespace game {

  class Enemy {
    public:
      /**
       * @brief Construct an Enemy wrapper that associates a game enemy with an ECS entity.
       *
       * @param enemy_id Unique game-level identifier for the enemy.
       * @param entity_id Identifier of the corresponding ECS entity.
       * @param ecsManager Reference to the ECS manager used to access the entity's components; the manager must outlive this Enemy instance.
       */
      Enemy(int enemy_id, std::uint32_t entity_id, ecs::ECSManager &ecsManager)
          : _enemy_id(enemy_id),
            _entity_id(entity_id),
            _ecsManager(ecsManager) {
      }
      ~Enemy() = default;

      /**
       * @brief Gets the enemy's unique identifier.
       *
       * @return int The enemy's identifier.
       */
      int getEnemyId() const {
        return _enemy_id;
      }

      /**
       * @brief Retrieve the entity's unique identifier within the ECS.
       *
       * @return std::uint32_t The ID of the entity associated with this Enemy.
       */
      std::uint32_t getEntityId() const {
        return _entity_id;
      }

      std::pair<float, float> getPosition() const;
      void setPosition(float x, float y);
      void move(float deltaX, float deltaY);
      std::optional<int> getHealth() const;
      std::optional<int> getMaxHealth() const;
      std::uint32_t getScore() const;
      void setHealth(int health);
      void takeDamage(int damage);
      void heal(int amount);
      bool isAlive() const;
      std::pair<float, float> getVelocity() const;
      void setVelocity(float vx, float vy);
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
      int _enemy_id;
      std::uint32_t _entity_id;
      ecs::ECSManager &_ecsManager;
  };

}  // namespace game