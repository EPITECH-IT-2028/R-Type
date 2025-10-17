#pragma once

#include <chrono>
#include <unordered_map>
#include "ECSManager.hpp"
#include "Packet.hpp"
#include "Queue.hpp"
#include "System.hpp"

namespace ecs {
  struct PlayerInput {
      MovementInputType input;
      int sequence_number;
      std::chrono::steady_clock::time_point timestamp;
  };

  class ServerInputSystem : public System {
    public:
      explicit ServerInputSystem();

      void setECSManager(ECSManager *ecsManager) {
        _ecsManagerPtr = ecsManager;
      }
      void setEventQueue(queue::EventQueue *eventQueue) {
        _eventQueue = eventQueue;
      }
      void update(float deltaTime) override;
      void validateAndApplyMovement(Entity entityId, float deltaTime);

      void queueInput(Entity entityId, const PlayerInput &input);
      void processInput(Entity entityId, const PlayerInput &input, float deltaTime);
      void sendPositionUpdate(Entity entityId);
    private:
      ECSManager *_ecsManagerPtr = nullptr;
      queue::EventQueue *_eventQueue = nullptr;
      std::unordered_map<Entity, std::vector<PlayerInput>> _pendingInputs;
      // std::unordered_map<Entity, PlayerInput> _pendingInputs;
      std::unordered_map<Entity, std::chrono::steady_clock::time_point>
          _lastInputTime;
  };
}  // namespace ecs
