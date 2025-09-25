#pragma once

#include <string>
#include <utility>
#include "ECSManager.hpp"

namespace game {

  class Player {
    public:
      Player(int player_id, uint32_t entity_id, ecs::ECSManager &ecsManager);
      ~Player() = default;

      int getPlayerId() const {
        return _player_id;
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

      float getSpeed() const;
      void setSpeed(float speed);

      std::pair<float, float> getVelocity() const;
      void setVelocity(float vx, float vy);

      uint32_t getSequenceNumber() const;
      void setSequenceNumber(uint32_t seq);
      bool isConnected() const;
      void setConnected(bool connected);

      const std::string &getName() const;
      void setName(const std::string &name);

      void update(float deltaTime);

    private:
      int _player_id;
      uint32_t _entity_id;
      ecs::ECSManager &_ecsManager;

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
  };

}  // namespace game
