#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class ProjectileSystem : public System {
    public:
      explicit ProjectileSystem(
          ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {
      }

      void setECSManager(ECSManager *ecsManager) {
        _ecsManagerPtr = ecsManager;
      }

      void update(float dt) override;
      void moveBasics(const Entity &entity, float dt);

    private:
      ECSManager &_ecsManager;
      ECSManager *_ecsManagerPtr = nullptr;
  };
}  // namespace ecs
