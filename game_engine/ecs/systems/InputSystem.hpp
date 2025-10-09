#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  // PlayerTiltFrame maps to player sprite-sheet frame indices.
  // These values must match the sprite layout used for player tilt animation.
  enum class PlayerTiltFrame {
    START = 0,    ///< Sprite frame for upward tilt
    NEUTRAL = 2,  ///< Sprite frame for neutral (no tilt)
    END = 4       ///< Sprite frame for downward tilt
  };
}  // namespace ecs

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
