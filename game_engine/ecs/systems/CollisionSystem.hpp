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
      /**
 * @brief Constructs a CollisionSystem and binds it to an ECS manager.
 *
 * Initializes the system with the provided ECSManager reference; if none is supplied,
 * the global singleton instance is used.
 *
 * @param ecsManager Reference to the ECSManager that the collision system will use.
 */
CollisionSystem(ECSManager &ecsManager = ECSManager::getInstance()) : _ecsManager(ecsManager) {};
      /**
 * @brief Destroys the CollisionSystem and performs any necessary cleanup.
 */
~CollisionSystem() override = default;

      /**
       * @brief Associates this CollisionSystem with a Game instance.
       *
       * @param game Pointer to the Game to associate with the system; may be nullptr to clear the association.
       */
      void setGame(game::Game *game) {
        _game = game;
      }

      /**
       * @brief Sets the EventQueue used by the collision system for dispatching events.
       *
       * @param eventQueue Pointer to the EventQueue to use; may be `nullptr` to disable event dispatch.
       */
      void setEventQueue(queue::EventQueue *eventQueue) {
        _eventQueue = eventQueue;
      }

      void update(float deltaTime) override;

      bool overlapAABBAABB(const Entity &a, const Entity &b) const;

      void handleCollision(const Entity &entity1, const Entity &entity2);

      void incrementPlayerScore(std::uint32_t owner_id, int value);

    private:
      ECSManager &_ecsManager;
      game::Game *_game = nullptr;
      queue::EventQueue *_eventQueue = nullptr;
  };
}  // namespace ecs
