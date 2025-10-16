#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class MovementSystem : public System {
    public:
      explicit MovementSystem(
          ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {}

      void setECSManager(ECSManager *ecsManager) {
        _ecsManagerPtr = ecsManager;
      }

      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager;
      ECSManager *_ecsManagerPtr = nullptr;
  };
}  // namespace ecs
