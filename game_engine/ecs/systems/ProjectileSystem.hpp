#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class ProjectileSystem : public System {
    public:
      ProjectileSystem() = default;
      ~ProjectileSystem() = default;

      void setECSManager(ECSManager *ecsManager) {
        _ecsManager = ecsManager;
      }
      void update(float dt);
      void moveBasics(float dt);

    private:
      ECSManager *_ecsManager;
  };
}  // namespace ecs
