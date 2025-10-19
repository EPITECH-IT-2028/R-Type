#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class MovementSystem : public System {
    public:
      explicit MovementSystem(
          ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {}

      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager;
  };
}  // namespace ecs
