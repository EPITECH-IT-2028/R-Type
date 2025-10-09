#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
    constexpr int PLAYER_TILT_START = 0;
    constexpr int PLAYER_TILT_NEUTRAL = 2;
    constexpr int PLAYER_TILT_END = 4;
}

namespace ecs {
  class InputSystem : public System {
    public:
      explicit InputSystem(ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {}

      void update(float deltaTime) override;

    private:
      ECSManager &_ecsManager;
  };
}  // namespace ecs
