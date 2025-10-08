#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class BoundarySystem : public System {
    public:
      explicit BoundarySystem(
          ecs::ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {}

      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager;
  };
}  // namespace ecs
