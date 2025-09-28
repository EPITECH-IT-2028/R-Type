#pragma once

#include "ECSManager.hpp"
#include "Queue.hpp"
#include "System.hpp"

namespace game {
  class Game;
}

namespace ecs {
  class EnemySystem : public System {
    public:
      EnemySystem() = default;
      ~EnemySystem() override = default;

      void setECSManager(ECSManager *ecsManager) {
        _ecsManager = ecsManager;
      }

      void setGame(game::Game *game) {
        _game = game;
      }

      void setEventQueue(queue::EventQueue *eventQueue) {
        _eventQueue = eventQueue;
      }

      void update(float deltaTime) override;

    private:
      ECSManager *_ecsManager = nullptr;
      game::Game *_game = nullptr;
      queue::EventQueue *_eventQueue = nullptr;

      void moveBasics(float deltaTime, const Entity &entity);
      void shootAtPlayer(float deltaTime, const Entity &entity);
      std::pair<float, float> findNearest(float x, float y);
  };
}  // namespace ecs
