#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class InputSystem : public System {
    public:
      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager = ECSManager::getInstance();
  };
}  // namespace ecs
