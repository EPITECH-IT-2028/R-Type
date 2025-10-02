#pragma once

#include "ECSManager.hpp"
#include "Queue.hpp"
#include "System.hpp"

namespace game {
  class Game;
}

namespace ecs {
  class CollisionSystem : public System {
    public:
      CollisionSystem(ECSManager &ecsManager = ECSManager::getInstance()) : _ecsManager(ecsManager) {};
      ~CollisionSystem() override = default;

      void setGame(game::Game *game) {
        _game = game;
      }

      void setEventQueue(queue::EventQueue *eventQueue) {
        _eventQueue = eventQueue;
      }

      void update(float deltaTime) override;

      bool overlapAABBAABB(const Entity &a, const Entity &b) const;

      void handleCollision(const Entity &entity1, const Entity &entity2);

    private:
      ECSManager &_ecsManager;
      game::Game *_game = nullptr;
      queue::EventQueue *_eventQueue = nullptr;
  };
}  // namespace ecs
