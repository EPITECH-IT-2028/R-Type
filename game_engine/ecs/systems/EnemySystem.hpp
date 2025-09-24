#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class EnemySystem : public System {
    public:
      EnemySystem() = default;
      ~EnemySystem() override = default;

      void setECSManager(ECSManager *ecsManager) {
        _ecsManager = ecsManager;
      }

      void update(float deltaTime) override;

    private:
      ECSManager *_ecsManager;
      void moveBasics(float deltaTime);
      void shootAtPlayer(float deltaTime);
  };
}  // namespace ecs
